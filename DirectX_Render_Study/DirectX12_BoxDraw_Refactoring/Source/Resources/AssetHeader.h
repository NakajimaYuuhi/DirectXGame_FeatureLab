#pragma once

#include <cstdint>
#include <cstring>

namespace AssetSecurity
{
    // マジックナンバー: 'A', 'G', 'Y', 'E' (AntiGravity Encrypted)
    constexpr uint32_t ASSET_MAGIC_ENCRYPTED = 0x45594741;
    // glTF Binary マジックナンバー: 'g', 'l', 'T', 'F' (0x46546C67)
    constexpr uint32_t GLTF_BINARY_MAGIC    = 0x46546C67;
    // 独自モデルバイナリ マジックナンバー: 'M', 'B', 'X', '1'
    constexpr uint32_t MBX_BINARY_MAGIC     = 0x3158424D;

    enum class AssetCipherType : uint16_t
    {
        None = 0,
        Xor  = 1,
        Aes  = 2
    };

#pragma pack(push, 1)
    struct AssetFileHeader
    {
        uint32_t magic;          // ASSET_MAGIC_ENCRYPTED
        uint16_t version;        // バージョン (初期値: 1)
        uint16_t cipherType;     // AssetCipherType
        uint64_t originalSize;   // 復号後の元データバイトサイズ
        uint64_t encryptedSize;  // 暗号化データのバイトサイズ
        uint32_t checksum;       // チェックサム
        uint8_t  reserved[8];    // 予約領域 (パディング)

        bool IsValid() const
        {
            return magic == ASSET_MAGIC_ENCRYPTED;
        }
    };
#pragma pack(pop)

    // 簡易チェックサム計算 (CRC32)
    inline uint32_t ComputeChecksum(const uint8_t* data, size_t size)
    {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < size; ++i)
        {
            crc ^= data[i];
            for (int k = 0; k < 8; ++k)
            {
                crc = (crc >> 1) ^ (0xEDB88320 & (-(int32_t)(crc & 1)));
            }
        }
        return ~crc;
    }
}
