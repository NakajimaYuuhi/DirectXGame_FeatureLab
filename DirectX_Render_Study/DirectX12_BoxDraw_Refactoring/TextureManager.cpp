#include "TextureManager.h"

std::shared_ptr<CTexture> TextureManager::GetTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const wchar_t* filePath, int srvIndex)
{
    std::wstring pathStr(filePath);

    auto it = m_textures.find(pathStr);
    if (it != m_textures.end())
    {
        return it->second;
    }

    // Load new texture
    std::shared_ptr<CTexture> newTexture = std::make_shared<CTexture>();
    if (newTexture->LoadTexture(device, cmdList, filePath, srvIndex))
    {
        newTexture->CreateSRV(device);
        m_textures[pathStr] = newTexture;
        return newTexture;
    }

    return nullptr;
}
