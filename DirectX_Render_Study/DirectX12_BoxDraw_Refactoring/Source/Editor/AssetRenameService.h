#pragma once
#include <string>
#include <vector>

struct RenameReferenceHit
{
	std::string filePath;
	int occurrences = 0;
};

struct RenamePlan
{
	std::string oldPath;
	std::string newPath;
	std::string oldName;       // Stem without extension
	std::string newName;       // Stem without extension
	std::string oldFileName;   // With extension (or folder name)
	std::string newFileName;   // With extension (or folder name)
	bool isDirectory = false;
	bool isPrefab = false;
	bool isScene = false;

	std::vector<RenameReferenceHit> hits;
	int totalOccurrences = 0;

	bool isValid = false;
	std::string errorMessage;
};

class AssetRenameService
{
public:
	static RenamePlan AnalyzeRename(const std::string& oldPath, const std::string& requestedNewName);
	static RenamePlan AnalyzeMove(const std::string& sourcePath, const std::string& targetFolder);
	static bool ExecuteRename(const RenamePlan& plan, bool updateReferences, std::string* outError = nullptr, bool recordUndo = true);

private:
	static void ScanForReferences(const RenamePlan& plan, std::vector<RenameReferenceHit>& outHits);
	static int ReplaceReferencesInContent(std::string& content, const RenamePlan& plan);
};
