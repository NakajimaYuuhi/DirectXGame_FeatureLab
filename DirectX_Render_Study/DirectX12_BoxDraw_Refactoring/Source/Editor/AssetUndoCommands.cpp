#include "AssetUndoCommands.h"
#include "SceneManager.h"
#include "PrefabManager.h"
#include "InspectorUI.h"
#include "ContentDrawerUI.h"
#include "HotReloadManager.h"
#include <filesystem>

namespace fs = std::filesystem;

static RenamePlan InvertRenamePlan(const RenamePlan& src)
{
	RenamePlan inv = src;
	inv.oldPath = src.newPath;
	inv.newPath = src.oldPath;
	inv.oldName = src.newName;
	inv.newName = src.oldName;
	inv.oldFileName = src.newFileName;
	inv.newFileName = src.oldFileName;
	return inv;
}

// ========================================================
// AssetRenameUndoCommand
// ========================================================

AssetRenameUndoCommand::AssetRenameUndoCommand(const RenamePlan& plan, bool updateReferences)
	: m_plan(plan), m_updateReferences(updateReferences)
{
}

void AssetRenameUndoCommand::Execute()
{
	// Redo rename
	std::string err;
	AssetRenameService::ExecuteRename(m_plan, m_updateReferences, &err, false);
}

void AssetRenameUndoCommand::Undo()
{
	// Revert rename
	RenamePlan inv = InvertRenamePlan(m_plan);
	std::string err;
	AssetRenameService::ExecuteRename(inv, m_updateReferences, &err, false);
}

std::string AssetRenameUndoCommand::GetName() const
{
	return "Rename Asset: " + m_plan.oldFileName + " -> " + m_plan.newFileName;
}

// ========================================================
// AssetDeleteUndoCommand
// ========================================================

AssetDeleteUndoCommand::AssetDeleteUndoCommand(
	const std::string& originalPath, const std::string& trashPath, const std::string& displayName)
	: m_originalPath(originalPath), m_trashPath(trashPath), m_displayName(displayName)
{
}

void AssetDeleteUndoCommand::Execute()
{
	// Redo delete: Move from originalPath to trashPath
	std::error_code ec;
	if (fs::exists(m_originalPath, ec))
	{
		fs::create_directories(fs::path(m_trashPath).parent_path(), ec);
		fs::rename(m_originalPath, m_trashPath, ec);
	}

	SceneManager::RefreshSceneRegistry();
	CContentDrawerUI::GetInstance().OnAssetDeleted(m_originalPath);
	HotReloadManager::GetInstance().CaptureSceneSnapshot();
}

void AssetDeleteUndoCommand::Undo()
{
	// Undo delete: Move from trashPath back to originalPath
	std::error_code ec;
	if (fs::exists(m_trashPath, ec))
	{
		fs::create_directories(fs::path(m_originalPath).parent_path(), ec);
		fs::rename(m_trashPath, m_originalPath, ec);
	}

	SceneManager::RefreshSceneRegistry();
	CContentDrawerUI::GetInstance().OnAssetRestored(m_originalPath);
	HotReloadManager::GetInstance().CaptureSceneSnapshot();
}

std::string AssetDeleteUndoCommand::GetName() const
{
	return "Delete Asset: " + m_displayName;
}
