#pragma once
#include <string>
#include <vector>

class CContentDrawerUI
{
public:
    static CContentDrawerUI& GetInstance()
    {
        static CContentDrawerUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

    void RefreshPrefabList();

private:
    CContentDrawerUI() = default;
    ~CContentDrawerUI() = default;
    CContentDrawerUI(const CContentDrawerUI&) = delete;
    CContentDrawerUI& operator=(const CContentDrawerUI&) = delete;

    struct PrefabItem
    {
        std::string name;
        std::string path;
        std::string tag;
    };

    bool m_isVisible = true;
    std::vector<PrefabItem> m_prefabItems;
    bool m_isInitialized = false;
    char m_searchFilter[128] = "";
};
