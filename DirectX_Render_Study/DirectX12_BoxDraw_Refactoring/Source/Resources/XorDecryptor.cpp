#include "XorDecryptor.h"
#include <cstdint>
#include <cstring>
#include <vector>
#include <string>

// --- RawDecryptor ---
bool RawDecryptor::Decrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData)
{
    if (!src || srcSize == 0) return false;
    outData.assign(src, src + srcSize);
    return true;
}

bool RawDecryptor::Encrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData)
{
    return Decrypt(src, srcSize, outData);
}

// --- XorDecryptor ---
static const uint8_t kDefaultXorKey[] = {
    0x58, 0x4F, 0x52, 0x5F, 0x4D, 0x4F, 0x44, 0x45, // "XOR_MODE"
    0x4C, 0x5F, 0x53, 0x45, 0x43, 0x55, 0x52, 0x45  // "L_SECURE"
};

XorDecryptor::XorDecryptor()
    : m_key(kDefaultXorKey, kDefaultXorKey + sizeof(kDefaultXorKey))
{
}

XorDecryptor::XorDecryptor(const std::vector<uint8_t>& key)
    : m_key(key)
{
    if (m_key.empty())
    {
        m_key.assign(kDefaultXorKey, kDefaultXorKey + sizeof(kDefaultXorKey));
    }
}

XorDecryptor::XorDecryptor(const std::string& keyString)
    : m_key(keyString.begin(), keyString.end())
{
    if (m_key.empty())
    {
        m_key.assign(kDefaultXorKey, kDefaultXorKey + sizeof(kDefaultXorKey));
    }
}

void XorDecryptor::ProcessXor(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData) const
{
    outData.resize(srcSize);
    const size_t keyLen = m_key.size();
    for (size_t i = 0; i < srcSize; ++i)
    {
        outData[i] = src[i] ^ m_key[i % keyLen];
    }
}

bool XorDecryptor::Decrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData)
{
    if (!src || srcSize == 0) return false;
    ProcessXor(src, srcSize, outData);
    return true;
}

bool XorDecryptor::Encrypt(const uint8_t* src, size_t srcSize, std::vector<uint8_t>& outData)
{
    if (!src || srcSize == 0) return false;
    ProcessXor(src, srcSize, outData);
    return true;
}
