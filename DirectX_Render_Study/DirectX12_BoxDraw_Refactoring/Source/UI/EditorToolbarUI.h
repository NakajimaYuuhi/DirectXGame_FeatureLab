#pragma once

class CEditorToolbarUI
{
public:
    static CEditorToolbarUI& GetInstance()
    {
        static CEditorToolbarUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

private:
    CEditorToolbarUI() = default;
    ~CEditorToolbarUI() = default;
    CEditorToolbarUI(const CEditorToolbarUI&) = delete;
    CEditorToolbarUI& operator=(const CEditorToolbarUI&) = delete;

    bool m_isVisible = true;
};
