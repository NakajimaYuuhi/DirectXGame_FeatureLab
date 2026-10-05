#pragma once

#include <string>
#include <vector>

class CAssetSecurityUI
{
public:
    static CAssetSecurityUI& GetInstance()
    {
        static CAssetSecurityUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

    void RefreshModelList();

private:
    CAssetSecurityUI() = default;
    ~CAssetSecurityUI() = default;
    CAssetSecurityUI(const CAssetSecurityUI&) = delete;
    CAssetSecurityUI& operator=(const CAssetSecurityUI&) = delete;

    struct ModelAssetInfo
    {
        std::string filename;
        std::string fullPath;
        uint64_t fileSize = 0;
        bool isEncrypted = false;
    };

    bool m_isVisible = false;
    bool m_isInitialized = false;
    std::vector<ModelAssetInfo> m_modelList;
    std::string m_statusMessage;
};
