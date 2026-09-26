#pragma once
#include <memory>
#include <vector>
#include <string>
#include <DirectXMath.h>

class CObject;

// Command Interface for Undo/Redo
class IUndoCommand
{
public:
    virtual ~IUndoCommand() = default;
    virtual void Execute() = 0;   // Redo / Apply action
    virtual void Undo() = 0;      // Undo action
    virtual std::string GetName() const = 0; // Command label for UI tooltip
};

// UndoManager Singleton
class UndoManager
{
public:
    static UndoManager& GetInstance()
    {
        static UndoManager instance;
        return instance;
    }

    void RecordCommand(std::unique_ptr<IUndoCommand> command);
    bool Undo();
    bool Redo();

    bool CanUndo() const { return !m_undoStack.empty(); }
    bool CanRedo() const { return !m_redoStack.empty(); }

    std::string GetUndoName() const;
    std::string GetRedoName() const;

    void Clear();

private:
    UndoManager() = default;
    ~UndoManager() = default;
    UndoManager(const UndoManager&) = delete;
    UndoManager& operator=(const UndoManager&) = delete;

    std::vector<std::unique_ptr<IUndoCommand>> m_undoStack;
    std::vector<std::unique_ptr<IUndoCommand>> m_redoStack;
    size_t m_maxHistorySize = 50;
};

// Built-in Command: Transform Change (Pos, Rotation, Scale)
class TransformUndoCommand : public IUndoCommand
{
public:
    TransformUndoCommand(
        CObject* target,
        const DirectX::XMFLOAT3& oldPos, const DirectX::XMFLOAT3& newPos,
        const DirectX::XMFLOAT3& oldRot, const DirectX::XMFLOAT3& newRot,
        const DirectX::XMFLOAT3& oldScale, const DirectX::XMFLOAT3& newScale
    );

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Transform Change"; }

private:
    CObject* m_targetObj = nullptr;
    DirectX::XMFLOAT3 m_oldPos;
    DirectX::XMFLOAT3 m_newPos;
    DirectX::XMFLOAT3 m_oldRot;
    DirectX::XMFLOAT3 m_newRot;
    DirectX::XMFLOAT3 m_oldScale;
    DirectX::XMFLOAT3 m_newScale;
};

// Built-in Command: Object Delete (Destroy/Restore)
class DeleteObjectUndoCommand : public IUndoCommand
{
public:
    DeleteObjectUndoCommand(CObject* target);

    void Execute() override;
    void Undo() override;
    std::string GetName() const override { return "Delete Object"; }

private:
    CObject* m_targetObj = nullptr;
};
