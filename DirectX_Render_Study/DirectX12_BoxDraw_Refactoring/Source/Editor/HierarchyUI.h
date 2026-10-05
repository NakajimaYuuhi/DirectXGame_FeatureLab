#pragma once
#include <unordered_map>
#include <utility>

class CObject;

class CHierarchyUI
{
public:
    static CHierarchyUI& GetInstance()
    {
        static CHierarchyUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

private:
    CHierarchyUI() = default;
    ~CHierarchyUI() = default;
    CHierarchyUI(const CHierarchyUI&) = delete;
    CHierarchyUI& operator=(const CHierarchyUI&) = delete;

    // 再帰的なノード描画
    void DrawObjectNode(CObject* obj, int tagIdx, int objIdx, const std::unordered_map<CObject*, std::pair<int, int>>& objIndexMap);

    bool m_isVisible = true;
};
