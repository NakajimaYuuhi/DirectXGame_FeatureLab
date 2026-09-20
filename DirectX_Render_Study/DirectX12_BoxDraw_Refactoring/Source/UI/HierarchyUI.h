#pragma once

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

    bool m_isVisible = true;
};
