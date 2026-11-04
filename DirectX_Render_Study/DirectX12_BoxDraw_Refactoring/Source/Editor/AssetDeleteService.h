#pragma once
#include <string>
#include <vector>

struct DeleteReferenceHit
{
	std::string filePath;
	int occurrences = 0;
};

struct DeletePlan
{
	std::string targetPath;          // e.g. "Assets/Scene/SceneTest.json"
	std::string fileName;            // Stem or full filename for display
	std::string stemName;            // Without extension
	bool isDirectory = false;
	bool isScene = false;
	bool isPrefab = false;

	std::vector<DeleteReferenceHit> hits;
	int totalOccurrences = 0;

	bool isValid = false;
	bool isBlocked = false;          // e.g. cannot delete active scene
	std::string errorMessage;
};

class AssetDeleteService
{
public:
	static DeletePlan AnalyzeDelete(const std::string& targetPath);
	static bool ExecuteDelete(const DeletePlan& plan, std::string* outError = nullptr, bool recordUndo = true);

private:
	static void ScanForReferences(const DeletePlan& plan, std::vector<DeleteReferenceHit>& outHits);
};
