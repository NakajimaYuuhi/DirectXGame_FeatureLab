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

private:
    CInspectorUI() = default;
    ~CInspectorUI() = default;
    CInspectorUI(const CInspectorUI&) = delete;
    CInspectorUI& operator=(const CInspectorUI&) = delete;

    // 選択されたオブジェクトなどの状態を保持するメンバ変数を追加可能
    int m_selectedObjectIndex = -1;
    int m_selectedTagIndex = -1;
    char m_shaderPathInput[256] = "Assets/Shader/Wireframe.hlsl";
};
