#pragma once
#include <string>
#include <memory>

class CObject;

class CInspectorUI
{
public:
    static CInspectorUI& GetInstance()
    {
        static CInspectorUI instance;
        return instance;
    }

    void Draw();

    bool IsPaused() const { return m_isPaused; }
    void SetPaused(bool paused) { m_isPaused = paused; }
    bool IsEditMode() const { return m_isEditMode; }
    void SetEditMode(bool editMode) { m_isEditMode = editMode; }
    bool ShouldShowColliders() const { return m_showColliders; }
    void SetShowColliders(bool show) { m_showColliders = show; }
    bool ShouldUpdateGame();

    int GetSelectedTagIndex() const { return m_selectedTagIndex; }
    int GetSelectedObjectIndex() const { return m_selectedObjectIndex; }
    void SetSelectedObject(int tagIdx, int objIdx) { m_selectedTagIndex = tagIdx; m_selectedObjectIndex = objIdx; }

    // Prefab Edit Mode (Prefab Stage)
    bool IsPrefabEditMode() const { return m_isPrefabEditMode; }
    std::string GetEditingPrefabPath() const { return m_editingPrefabPath; }
    CObject* GetPrefabEditTarget() const { return m_prefabEditTarget.get(); }

    void OpenPrefabEditMode(const std::string& jsonPath);
    void ClosePrefabEditMode();
    bool SaveCurrentPrefab();

private:
    CInspectorUI() = default;
    ~CInspectorUI() = default;
    CInspectorUI(const CInspectorUI&) = delete;
    CInspectorUI& operator=(const CInspectorUI&) = delete;

    int m_selectedObjectIndex = -1;
    int m_selectedTagIndex = -1;
    char m_shaderPathInput[256] = "Assets/Shader/Wireframe.hlsl";
    char m_sceneJsonPath[256] = "Assets/Scene/SceneTest.json";
    bool m_showColliders = true;

    bool m_isEditMode = true;
    bool m_isPaused = false;
    bool m_stepNextFrame = false;
    float m_timeScale = 1.0f;

    // Prefab Edit Mode State
    bool m_isPrefabEditMode = false;
    std::string m_editingPrefabPath = "";
    std::unique_ptr<CObject> m_prefabEditTarget = nullptr;
};
