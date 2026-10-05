#pragma once

class CProfilerUI
{
public:
    static CProfilerUI& GetInstance()
    {
        static CProfilerUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

private:
    CProfilerUI() = default;
    ~CProfilerUI() = default;
    CProfilerUI(const CProfilerUI&) = delete;
    CProfilerUI& operator=(const CProfilerUI&) = delete;

    bool m_isVisible = false;
};