#pragma once

#include "AssetHeader.h"
#include <cstdint>
#include <vector>
#include <memory>

class IAssetDecryptor
{
public:
    virtual ~IAssetDecryptor() = default;

    virtual bool Decrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) = 0;
    virtual bool Encrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) = 0;
    virtual AssetSecurity::AssetCipherType GetCipherType() const = 0;
};
