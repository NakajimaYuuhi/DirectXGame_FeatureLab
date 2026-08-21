#pragma once

#include "Texture.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <d3d12.h>

class TextureManager
{
public:
    static TextureManager& GetInstance()
    {
        static TextureManager instance;
        return instance;
    }

    std::shared_ptr<CTexture> GetTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const wchar_t* filePath);
	void Clear() { m_textures.clear(); }

private:
    TextureManager() = default;
    ~TextureManager() = default;
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    std::unordered_map<std::wstring, std::shared_ptr<CTexture>> m_textures;
};
