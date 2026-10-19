#pragma once

#include "GamePackageBuilder.h"

class CBuildSettingsUI
{
public:
    static CBuildSettingsUI& GetInstance()
    {
        static CBuildSettingsUI instance;
        return instance;
    }

    void Draw();

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

private:
    CBuildSettingsUI() = default;
    ~CBuildSettingsUI() = default;
    CBuildSettingsUI(const CBuildSettingsUI&) = delete;
    CBuildSettingsUI& operator=(const CBuildSettingsUI&) = delete;

    bool m_isVisible = false;
    bool m_autoScrollLog = true;
    int m_configIndex = 0; // 0: Release, 1: Debug
    PackageBuildConfig m_packageConfig;
};
