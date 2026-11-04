#pragma once
#include "UndoManager.h"
#include "AssetRenameService.h"
#include <string>

// Undo/Redo command for Asset Rename (including reference rewrites)
class AssetRenameUndoCommand : public IUndoCommand
{
public:
	AssetRenameUndoCommand(const RenamePlan& plan, bool updateReferences);

	void Execute() override; // Redo
	void Undo() override;    // Rollback to original name and references
	std::string GetName() const override;

private:
	RenamePlan m_plan;
	bool m_updateReferences;
};

// Undo/Redo command for Asset Delete (restoring from trash)
class AssetDeleteUndoCommand : public IUndoCommand
{
public:
	AssetDeleteUndoCommand(const std::string& originalPath, const std::string& trashPath, const std::string& displayName);

	void Execute() override; // Redo: Move back to trash
	void Undo() override;    // Move back to original path
	std::string GetName() const override;

private:
	std::string m_originalPath;
	std::string m_trashPath;
	std::string m_displayName;
};
