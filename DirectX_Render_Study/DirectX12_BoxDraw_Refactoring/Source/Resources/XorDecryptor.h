#pragma once

#include "IAssetDecryptor.h"
#include <cstdint>
#include <vector>
#include <string>

// パススルー (平文)
class RawDecryptor : public IAssetDecryptor
{
public:
    RawDecryptor() = default;
    ~RawDecryptor() override = default;

    bool Decrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) override;
    bool Encrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) override;
    AssetSecurity::AssetCipherType GetCipherType() const override { return AssetSecurity::AssetCipherType::None; }
};

// XOR 暗号化 / 復号器
class XorDecryptor : public IAssetDecryptor
{
public:
    XorDecryptor();
    explicit XorDecryptor(const std::vector<uint8_t>& key);
    explicit XorDecryptor(const std::string& keyString);
    ~XorDecryptor() override = default;

    bool Decrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) override;
    bool Encrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) override;
    AssetSecurity::AssetCipherType GetCipherType() const override { return AssetSecurity::AssetCipherType::Xor; }

    const std::vector<uint8_t>& GetKey() const { return m_key; }

private:
    void ProcessXor(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) const;

    std::vector<uint8_t> m_key;
};
