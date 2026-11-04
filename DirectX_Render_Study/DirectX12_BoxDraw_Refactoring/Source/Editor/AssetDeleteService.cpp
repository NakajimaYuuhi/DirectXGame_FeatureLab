#include "AssetDeleteService.h"
#include "AssetUndoCommands.h"
#include "UndoManager.h"
#include "SceneManager.h"
#include "PrefabManager.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "HotReloadManager.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <chrono>

namespace fs = std::filesystem;

static std::string NormalizePath(const std::string& raw)
{
	std::string p = raw;
	std::replace(p.begin(), p.end(), '\\', '/');
	return p;
}

DeletePlan AssetDeleteService::AnalyzeDelete(const std::string& targetPath)
{
	DeletePlan plan;
	plan.targetPath = NormalizePath(targetPath);

	std::error_code ec;
	if (plan.targetPath.empty() || !fs::exists(plan.targetPath, ec))
	{
		plan.isValid = false;
		plan.errorMessage = "Asset file or folder does not exist.";
		return plan;
	}

	plan.isDirectory = fs::is_directory(plan.targetPath, ec);
	plan.fileName = fs::path(plan.targetPath).filename().string();
	plan.stemName = fs::path(plan.targetPath).stem().string();

	std::string ext = fs::path(plan.targetPath).extension().string();
	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

	plan.isScene = (!plan.isDirectory && ext == ".json" && plan.targetPath.find("Assets/Scene") != std::string::npos);
	plan.isPrefab = (!plan.isDirectory && ext == ".json" && (plan.targetPath.find("Assets/Prefab") != std::string::npos || plan.targetPath.find("Assets/Prefabs") != std::string::npos));

	// Guard: Cannot delete the currently active scene
	if (plan.isScene)
	{
		std::string activeScenePath = NormalizePath(SceneManager::GetInstance().GetActiveScenePath());
		if (activeScenePath == plan.targetPath)
		{
			plan.isValid = false;
			plan.isBlocked = true;
			plan.errorMessage = "Cannot delete the currently active scene. Please load another scene first.";
			return plan;
		}
	}

	// Scan for references across all asset files
	ScanForReferences(plan, plan.hits);
	for (const auto& hit : plan.hits)
	{
		plan.totalOccurrences += hit.occurrences;
	}

	plan.isValid = true;
	return plan;
}

void AssetDeleteService::ScanForReferences(const DeletePlan& plan, std::vector<DeleteReferenceHit>& outHits)
{
	outHits.clear();
	std::error_code ec;
	fs::path assetsDir("Assets");
	if (!fs::exists(assetsDir, ec)) return;

	std::string forwardPath = plan.targetPath;
	std::string backslashPath = plan.targetPath;
	std::replace(backslashPath.begin(), backslashPath.end(), '/', '\\');

	std::string forwardDirSlash = plan.isDirectory ? (forwardPath + "/") : "";
	std::string backslashDirSlash = plan.isDirectory ? (backslashPath + "\\") : "";

	std::string prefabKeyPattern = plan.isPrefab ? ("\"prefab\": \"" + plan.stemName + "\"") : "";

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
		if (scanFilePath == plan.targetPath) continue;

		std::ifstream file(scanFilePath, std::ios::binary);
		if (!file.is_open()) continue;

		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();

		int matchCount = 0;
		if (plan.isDirectory)
		{
			size_t pos = 0;
			while ((pos = content.find(forwardDirSlash, pos)) != std::string::npos) { matchCount++; pos += forwardDirSlash.length(); }
			pos = 0;
			while ((pos = content.find(backslashDirSlash, pos)) != std::string::npos) { matchCount++; pos += backslashDirSlash.length(); }
		}
		else
		{
			size_t pos = 0;
			while ((pos = content.find(forwardPath, pos)) != std::string::npos) { matchCount++; pos += forwardPath.length(); }
			pos = 0;
			while ((pos = content.find(backslashPath, pos)) != std::string::npos) { matchCount++; pos += backslashPath.length(); }
			if (!prefabKeyPattern.empty())
			{
				pos = 0;
				while ((pos = content.find(prefabKeyPattern, pos)) != std::string::npos) { matchCount++; pos += prefabKeyPattern.length(); }
			}
		}

		if (matchCount > 0)
		{
			outHits.push_back({ scanFilePath, matchCount });
		}
	}
}

bool AssetDeleteService::ExecuteDelete(const DeletePlan& plan, std::string* outError, bool recordUndo)
{
	std::error_code ec;
	if (!fs::exists(plan.targetPath, ec))
	{
		if (outError) *outError = "File or directory does not exist: " + plan.targetPath;
		return false;
	}

	// Prepare trash directory
	fs::path trashRoot(".trash");
	fs::create_directories(trashRoot, ec);

	auto now = std::chrono::system_clock::now();
	auto epochMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
	std::string trashName = std::to_string(epochMs) + "_" + plan.fileName;
	fs::path trashPath = trashRoot / trashName;

	// Move target to trash
	fs::rename(plan.targetPath, trashPath, ec);
	if (ec)
	{
		if (outError) *outError = "Failed to move asset to trash: " + ec.message();
		return false;
	}

	// Notify engine subsystems
	SceneManager::RefreshSceneRegistry();
	CContentDrawerUI::GetInstance().OnAssetDeleted(plan.targetPath);
	HotReloadManager::GetInstance().CaptureSceneSnapshot();

	// Record Undo Command
	if (recordUndo)
	{
		UndoManager::GetInstance().RecordCommand(
			std::make_unique<AssetDeleteUndoCommand>(plan.targetPath, trashPath.generic_string(), plan.fileName));
	}

	return true;
}
