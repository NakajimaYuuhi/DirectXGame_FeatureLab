#pragma once
#include <string>

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
    bool IsEditMode() const { return m_isEditMode; }
    bool ShouldUpdateGame();

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

    bool m_isEditMode = false;
    bool m_isPaused = false;
    bool m_stepNextFrame = false;
    float m_timeScale = 1.0f;
};