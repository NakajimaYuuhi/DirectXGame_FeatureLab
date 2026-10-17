#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <d3d12.h>
#include "RenderTexture.h"

// ==============================================================================
// RenderBlackboard
// Central repository for sharing textures/buffers between passes
// ==============================================================================
class RenderBlackboard {
public:
    RenderBlackboard() = default;
    ~RenderBlackboard() = default;

    // Register owned texture
    void RegisterTexture(const std::string& name, std::shared_ptr<RenderTexture> texture);

    // Register external unowned texture reference
    void RegisterTextureRef(const std::string& name, RenderTexture* texture);

    // Get texture (from either owned or referenced textures, returns nullptr if not found)
    RenderTexture* GetTexture(const std::string& name) const;

    // Check if texture exists
    bool HasTexture(const std::string& name) const;
    bool IsOwnedTexture(const std::string& name) const { return m_ownedTextures.find(name) != m_ownedTextures.end(); }

    // Unregister texture
    void UnregisterTexture(const std::string& name);

    // On-demand creation (creates and caches texture if it does not exist)
    RenderTexture* CreateOrGetTexture(
        const std::string& name,
        ID3D12Device* pDevice,
        UINT width,
        UINT height,
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM,
        const float clearColor[4] = nullptr,
        bool allowUAV = false
    );

    // Get all registered texture names
    std::vector<std::string> GetTextureNames() const;

    // Clear all
    void Clear();

private:
    std::unordered_map<std::string, std::shared_ptr<RenderTexture>> m_ownedTextures;
    std::unordered_map<std::string, RenderTexture*> m_textureRefs;
};
