#include "RenderBlackboard.h"
#include <algorithm>

void RenderBlackboard::RegisterTexture(const std::string& name, std::shared_ptr<RenderTexture> texture)
{
    if (name.empty() || !texture) return;
    m_textureRefs.erase(name);
    m_ownedTextures[name] = texture;
}

void RenderBlackboard::RegisterTextureRef(const std::string& name, RenderTexture* texture)
{
    if (name.empty() || !texture) return;
    m_ownedTextures.erase(name);
    m_textureRefs[name] = texture;
}

RenderTexture* RenderBlackboard::GetTexture(const std::string& name) const
{
    if (name.empty()) return nullptr;

    auto itOwned = m_ownedTextures.find(name);
    if (itOwned != m_ownedTextures.end()) {
        return itOwned->second.get();
    }

    auto itRef = m_textureRefs.find(name);
    if (itRef != m_textureRefs.end()) {
        return itRef->second;
    }

    return nullptr;
}

bool RenderBlackboard::HasTexture(const std::string& name) const
{
    if (name.empty()) return false;
    return m_ownedTextures.find(name) != m_ownedTextures.end() ||
           m_textureRefs.find(name) != m_textureRefs.end();
}

void RenderBlackboard::UnregisterTexture(const std::string& name)
{
    m_ownedTextures.erase(name);
    m_textureRefs.erase(name);
}

RenderTexture* RenderBlackboard::CreateOrGetTexture(
    const std::string& name,
    ID3D12Device* pDevice,
    UINT width,
    UINT height,
    DXGI_FORMAT format,
    const float clearColor[4],
    bool allowUAV)
{
    if (name.empty()) return nullptr;

    RenderTexture* existing = GetTexture(name);
    if (existing) {
        return existing;
    }

    if (!pDevice || width == 0 || height == 0) {
        return nullptr;
    }

    auto newTex = std::make_shared<RenderTexture>(pDevice, width, height, format, clearColor, allowUAV);
    RegisterTexture(name, newTex);
    return newTex.get();
}

std::vector<std::string> RenderBlackboard::GetTextureNames() const
{
    std::vector<std::string> names;
    names.reserve(m_ownedTextures.size() + m_textureRefs.size());

    for (const auto& pair : m_ownedTextures) {
        names.push_back(pair.first);
    }
    for (const auto& pair : m_textureRefs) {
        names.push_back(pair.first);
    }

    std::sort(names.begin(), names.end());
    return names;
}

void RenderBlackboard::Clear()
{
    m_ownedTextures.clear();
    m_textureRefs.clear();
}
