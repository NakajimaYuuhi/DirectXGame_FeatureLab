#include "UndoManager.h"
#include "Object.h"
#include "Transform.h"

void UndoManager::RecordCommand(std::unique_ptr<IUndoCommand> command)
{
    if (!command) return;

    m_undoStack.push_back(std::move(command));
    if (m_undoStack.size() > m_maxHistorySize)
    {
        m_undoStack.erase(m_undoStack.begin());
    }

    // New action invalidates the redo stack
    m_redoStack.clear();
}

bool UndoManager::Undo()
{
    if (m_undoStack.empty()) return false;

    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    cmd->Undo();
    m_redoStack.push_back(std::move(cmd));
    return true;
}

bool UndoManager::Redo()
{
    if (m_redoStack.empty()) return false;

    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    cmd->Execute();
    m_undoStack.push_back(std::move(cmd));
    return true;
}

std::string UndoManager::GetUndoName() const
{
    if (m_undoStack.empty()) return "";
    return m_undoStack.back()->GetName();
}

std::string UndoManager::GetRedoName() const
{
    if (m_redoStack.empty()) return "";
    return m_redoStack.back()->GetName();
}

void UndoManager::Clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}

// TransformUndoCommand Implementation
TransformUndoCommand::TransformUndoCommand(
    CObject* target,
    const DirectX::XMFLOAT3& oldPos, const DirectX::XMFLOAT3& newPos,
    const DirectX::XMFLOAT3& oldRot, const DirectX::XMFLOAT3& newRot,
    const DirectX::XMFLOAT3& oldScale, const DirectX::XMFLOAT3& newScale)
    : m_targetObj(target)
    , m_oldPos(oldPos), m_newPos(newPos)
    , m_oldRot(oldRot), m_newRot(newRot)
    , m_oldScale(oldScale), m_newScale(newScale)
{
}

void TransformUndoCommand::Execute()
{
    if (!m_targetObj || m_targetObj->GetIsDestroyed()) return;
    CTransform* transform = m_targetObj->GetComponent<CTransform>();
    if (transform)
    {
        transform->SetPos(m_newPos);
        transform->SetRotation(m_newRot);
        transform->SetScale(m_newScale);
    }
}

void TransformUndoCommand::Undo()
{
    if (!m_targetObj || m_targetObj->GetIsDestroyed()) return;
    CTransform* transform = m_targetObj->GetComponent<CTransform>();
    if (transform)
    {
        transform->SetPos(m_oldPos);
        transform->SetRotation(m_oldRot);
        transform->SetScale(m_oldScale);
    }
}

// DeleteObjectUndoCommand Implementation
DeleteObjectUndoCommand::DeleteObjectUndoCommand(CObject* target)
    : m_targetObj(target)
{
}

void DeleteObjectUndoCommand::Execute()
{
    if (m_targetObj)
    {
        m_targetObj->SetIsDestroyed(true);
    }
}

void DeleteObjectUndoCommand::Undo()
{
    if (m_targetObj)
    {
        m_targetObj->SetIsDestroyed(false);
    }
}
