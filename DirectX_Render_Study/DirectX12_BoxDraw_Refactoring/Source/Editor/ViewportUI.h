#pragma once
#include <d3d12.h>
#include "imgui.h"

class CViewportUI
{
public:
    static CViewportUI& GetInstance()
    {
        static CViewportUI instance;
        return instance;
    }

    void Draw();

    void SetTextureSRV(D3D12_GPU_DESCRIPTOR_HANDLE srvHandle)
    {
        m_textureSRV = srvHandle;
        m_isTextureAvailable = (srvHandle.ptr != 0);
    }

    void SetTextureAvailable(bool available) { m_isTextureAvailable = available; }

    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }
    void ToggleVisible() { m_isVisible = !m_isVisible; }

    // ビューポート状態
    ImVec2 GetViewportPos() const { return m_viewportPos; }
    ImVec2 GetViewportSize() const { return m_viewportSize; }
    ImVec2 GetImagePos() const { return m_imagePos; }
    ImVec2 GetImageSize() const { return m_imageSize; }
    bool IsHovered() const { return m_isHovered; }
    bool IsFocused() const { return m_isFocused; }

    // ビューポート画像内の正規化マウス座標 (0.0f ~ 1.0f) を取得。画像外なら false
    bool GetNormalizedMousePos(float& outX, float& outY) const;

private:
    CViewportUI() = default;
    ~CViewportUI() = default;
    CViewportUI(const CViewportUI&) = delete;
    CViewportUI& operator=(const CViewportUI&) = delete;

    bool m_isVisible = true;
    bool m_isTextureAvailable = false;
    D3D12_GPU_DESCRIPTOR_HANDLE m_textureSRV = {};

    ImVec2 m_viewportPos = { 0.0f, 0.0f };
    ImVec2 m_viewportSize = { 0.0f, 0.0f };
    ImVec2 m_imagePos = { 0.0f, 0.0f };
    ImVec2 m_imageSize = { 0.0f, 0.0f };
    bool m_isHovered = false;
    bool m_isFocused = false;
};
