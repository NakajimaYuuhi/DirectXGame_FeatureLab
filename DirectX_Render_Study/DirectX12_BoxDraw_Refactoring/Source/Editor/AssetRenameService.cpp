#include "AssetRenameService.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include "Source/Core/PrefabManager.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "HotReloadManager.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <algorithm>

namespace fs = std::filesystem;

static std::string NormalizePath(const std::string& path)
{
	std::string p = path;
	for (auto& c : p)
	{
		if (c == '\\') c = '/';
	}
	while (!p.empty() && p.back() == '/')
	{
		p.pop_back();
	}
	return p;
}

static std::string ToBackslash(const std::string& path)
{
	std::string p = path;
	for (auto& c : p)
	{
		if (c == '/') c = '\\';
	}
	return p;
}

static int ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty() || from == to) return 0;
	int count = 0;
	size_t pos = 0;
	while ((pos = str.find(from, pos)) != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
		++count;
	}
	return count;
}

RenamePlan AssetRenameService::AnalyzeRename(const std::string& oldPath, const std::string& requestedNewName)
{
	RenamePlan plan;
	plan.oldPath = NormalizePath(oldPath);

	std::error_code ec;
	if (!fs::exists(plan.oldPath, ec))
	{
		plan.isValid = false;
		plan.errorMessage = "Source asset does not exist: " + plan.oldPath;
		return plan;
	}

	std::string trimmedName = requestedNewName;
	while (!trimmedName.empty() && (trimmedName.front() == ' ' || trimmedName.front() == '\t'))
	{
		trimmedName.erase(trimmedName.begin());
	}
	while (!trimmedName.empty() && (trimmedName.back() == ' ' || trimmedName.back() == '\t'))
	{
		trimmedName.pop_back();
	}

	if (trimmedName.empty())
	{
		plan.isValid = false;
		plan.errorMessage = "Asset name cannot be empty.";
		return plan;
	}

	const std::string invalidChars = ":*?\"<>|";
	for (char c : invalidChars)
	{
		if (trimmedName.find(c) != std::string::npos)
		{
			plan.isValid = false;
			plan.errorMessage = "Name contains invalid character: " + std::string(1, c);
			return plan;
		}
	}
	if (trimmedName.find('/') != std::string::npos || trimmedName.find('\\') != std::string::npos)
	{
		plan.isValid = false;
		plan.errorMessage = "Name cannot contain directory separators.";
		return plan;
	}

	plan.isDirectory = fs::is_directory(plan.oldPath, ec);
	fs::path oldFsPath(plan.oldPath);
	std::string parentDir = oldFsPath.parent_path().generic_string();

	if (plan.isDirectory)
	{
		plan.oldFileName = oldFsPath.filename().string();
		plan.newFileName = trimmedName;
		plan.oldName = plan.oldFileName;
		plan.newName = plan.newFileName;
		plan.newPath = parentDir.empty() ? plan.newFileName : (parentDir + "/" + plan.newFileName);
	}
	else
	{
		plan.oldFileName = oldFsPath.filename().string();
		plan.oldName = oldFsPath.stem().string();
		std::string oldExt = oldFsPath.extension().string();

		fs::path requestedFs(trimmedName);
		std::string reqExt = requestedFs.extension().string();
		if (reqExt.empty())
		{
			plan.newFileName = trimmedName + oldExt;
			plan.newName = trimmedName;
		}
		else
		{
			plan.newFileName = trimmedName;
			plan.newName = requestedFs.stem().string();
		}

		plan.newPath = parentDir.empty() ? plan.newFileName : (parentDir + "/" + plan.newFileName);

		std::string lowerOld = plan.oldPath;
		std::transform(lowerOld.begin(), lowerOld.end(), lowerOld.begin(), ::tolower);
		if (oldExt == ".json" && lowerOld.find("scene") != std::string::npos)
		{
			plan.isScene = true;
		}
		else if (oldExt == ".json" && lowerOld.find("prefab") != std::string::npos)
		{
			plan.isPrefab = true;
		}
	}

	if (plan.oldPath == plan.newPath)
	{
		plan.isValid = false;
		plan.errorMessage = "New name is identical to the current name.";
		return plan;
	}

	if (fs::exists(plan.newPath, ec))
	{
		plan.isValid = false;
		plan.errorMessage = "An asset with the name '" + plan.newFileName + "' already exists.";
		return plan;
	}

	ScanForReferences(plan, plan.hits);
	for (const auto& hit : plan.hits)
	{
		plan.totalOccurrences += hit.occurrences;
	}

	plan.isValid = true;
	return plan;
}

void AssetRenameService::ScanForReferences(const RenamePlan& plan, std::vector<RenameReferenceHit>& outHits)
{
	outHits.clear();
	std::error_code ec;
	fs::path assetsDir("Assets");
	if (!fs::exists(assetsDir, ec)) return;

	for (const auto& entry : fs::recursive_directory_iterator(assetsDir, ec))
	{
		if (!entry.is_regular_file(ec)) continue;

		std::string ext = entry.path().extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

		if (ext != ".json" && ext != ".mat" && ext != ".pipeline" && ext != ".hlsl" && ext != ".h" && ext != ".cpp")
		{
			continue;
		}

		std::string scanFilePath = entry.path().generic_string();
		if (scanFilePath == plan.oldPath) continue;

		std::ifstream file(scanFilePath, std::ios::binary);
		if (!file.is_open()) continue;

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		int matchCount = ReplaceReferencesInContent(content, plan);
		if (matchCount > 0)
		{
			outHits.push_back({ scanFilePath, matchCount });
		}
	}
}

int AssetRenameService::ReplaceReferencesInContent(std::string& content, const RenamePlan& plan)
{
	int count = 0;

	if (plan.isDirectory)
	{
		std::string oldSlash = plan.oldPath + "/";
		std::string newSlash = plan.newPath + "/";
		count += ReplaceAll(content, oldSlash, newSlash);

		std::string oldBack = ToBackslash(plan.oldPath) + "\\";
		std::string newBack = ToBackslash(plan.newPath) + "\\";
		count += ReplaceAll(content, oldBack, newBack);

		count += ReplaceAll(content, "\"" + plan.oldPath + "\"", "\"" + plan.newPath + "\"");
		count += ReplaceAll(content, "\"" + ToBackslash(plan.oldPath) + "\"", "\"" + ToBackslash(plan.newPath) + "\"");
	}
	else
	{
		count += ReplaceAll(content, "\"" + plan.oldPath + "\"", "\"" + plan.newPath + "\"");
		count += ReplaceAll(content, "\"" + ToBackslash(plan.oldPath) + "\"", "\"" + ToBackslash(plan.newPath) + "\"");
		count += ReplaceAll(content, plan.oldPath, plan.newPath);

		if (plan.isPrefab && !plan.oldName.empty() && !plan.newName.empty())
		{
			count += ReplaceAll(content, "\"prefab\": \"" + plan.oldName + "\"", "\"prefab\": \"" + plan.newName + "\"");
			count += ReplaceAll(content, "\"prefab\":\"" + plan.oldName + "\"", "\"prefab\":\"" + plan.newName + "\"");
			count += ReplaceAll(content, "\"PrefabName\": \"" + plan.oldName + "\"", "\"PrefabName\": \"" + plan.newName + "\"");
			count += ReplaceAll(content, "\"PrefabName\":\"" + plan.oldName + "\"", "\"PrefabName\":\"" + plan.newName + "\"");
		}
	}

	return count;
}

bool AssetRenameService::ExecuteRename(const RenamePlan& plan, bool updateReferences, std::string* outError)
{
	if (!plan.isValid)
	{
		if (outError) *outError = plan.errorMessage.empty() ? "Invalid rename plan." : plan.errorMessage;
		return false;
	}

	std::error_code ec;
	std::vector<std::string> backedUpFiles;

	auto Rollback = [&]() {
		for (const auto& path : backedUpFiles)
		{
			std::string bakPath = path + ".bak";
			if (fs::exists(bakPath, ec))
			{
				fs::copy_file(bakPath, path, fs::copy_options::overwrite_existing, ec);
				fs::remove(bakPath, ec);
			}
		}
		if (fs::exists(plan.newPath, ec) && !fs::exists(plan.oldPath, ec))
		{
			fs::rename(plan.newPath, plan.oldPath, ec);
		}
	};

	if (updateReferences && !plan.hits.empty())
	{
		for (const auto& hit : plan.hits)
		{
			std::string bakPath = hit.filePath + ".bak";
			fs::copy_file(hit.filePath, bakPath, fs::copy_options::overwrite_existing, ec);
			if (ec)
			{
				if (outError) *outError = "Failed to create backup for " + hit.filePath;
				Rollback();
				return false;
			}
			backedUpFiles.push_back(hit.filePath);

			std::ifstream inFile(hit.filePath, std::ios::binary);
			if (!inFile.is_open())
			{
				if (outError) *outError = "Failed to read referencing file: " + hit.filePath;
				Rollback();
				return false;
			}
			std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
			inFile.close();

			ReplaceReferencesInContent(content, plan);

			std::ofstream outFile(hit.filePath, std::ios::binary | std::ios::trunc);
			if (!outFile.is_open())
			{
				if (outError) *outError = "Failed to write updated references to: " + hit.filePath;
				Rollback();
				return false;
			}
			outFile.write(content.data(), content.size());
			outFile.close();
		}
	}

	if (!plan.isDirectory)
	{
		std::ifstream selfFile(plan.oldPath, std::ios::binary);
		if (selfFile.is_open())
		{
			std::string selfContent((std::istreambuf_iterator<char>(selfFile)), std::istreambuf_iterator<char>());
			selfFile.close();

			int selfModified = 0;
			if (plan.isPrefab)
			{
				selfModified += ReplaceAll(selfContent, "\"PrefabName\": \"" + plan.oldName + "\"", "\"PrefabName\": \"" + plan.newName + "\"");
				selfModified += ReplaceAll(selfContent, "\"PrefabName\":\"" + plan.oldName + "\"", "\"PrefabName\":\"" + plan.newName + "\"");
			}
			if (plan.isScene)
			{
				selfModified += ReplaceAll(selfContent, "\"name\": \"" + plan.oldName + "\"", "\"name\": \"" + plan.newName + "\"");
				selfModified += ReplaceAll(selfContent, "\"name\":\"" + plan.oldName + "\"", "\"name\":\"" + plan.newName + "\"");
			}

			if (selfModified > 0)
			{
				std::ofstream outSelf(plan.oldPath, std::ios::binary | std::ios::trunc);
				if (outSelf.is_open())
				{
					outSelf.write(selfContent.data(), selfContent.size());
					outSelf.close();
				}
			}
		}
	}

	fs::rename(plan.oldPath, plan.newPath, ec);
	if (ec)
	{
		if (outError) *outError = "Filesystem rename failed: " + ec.message();
		Rollback();
		return false;
	}

	for (const auto& path : backedUpFiles)
	{
		std::string bakPath = path + ".bak";
		fs::remove(bakPath, ec);
	}

	SceneManager::GetInstance().OnAssetRenamed(plan.oldPath, plan.newPath);
	PrefabManager::GetInstance().OnAssetRenamed(plan.oldPath, plan.newPath);
	CInspectorUI::GetInstance().OnAssetRenamed(plan.oldPath, plan.newPath);
	CContentDrawerUI::GetInstance().OnAssetRenamed(plan.oldPath, plan.newPath);
	HotReloadManager::GetInstance().CaptureSceneSnapshot();

	return true;
}
