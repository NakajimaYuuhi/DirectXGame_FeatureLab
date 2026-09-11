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
    bool ShouldUpdateGame();

private:
    CInspectorUI() = default;
    ~CInspectorUI() = default;
    CInspectorUI(const CInspectorUI&) = delete;
    CInspectorUI& operator=(const CInspectorUI&) = delete;

    int m_selectedObjectIndex = -1;
    int m_selectedTagIndex = -1;
    char m_shaderPathInput[256] = "Assets/Shader/Wireframe.hlsl";
    bool m_showColliders = true;

    bool m_isPaused = false;
    bool m_stepNextFrame = false;
    float m_timeScale = 1.0f;
};