#include "AssetEncryptor.h"
#include "AssetHeader.h"
#include "XorDecryptor.h"
#include <fstream>
#include <vector>
#include <iostream>

namespace AssetSecurity
{
    bool EncryptModelFile(const std::string& inputPath, const std::string& outputPath)
    {
        std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
        if (!inFile.is_open())
        {
            std::cout << "[AssetEncryptor] Failed to open input file: " << inputPath << std::endl;
            return false;
        }

        std::streamsize fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        std::vector<uint8_t> rawData(static_cast<size_t>(fileSize));
        if (!inFile.read(reinterpret_cast<char*>(rawData.data()), fileSize))
        {
            std::cout << "[AssetEncryptor] Failed to read input file: " << inputPath << std::endl;
            return false;
        }

        XorDecryptor encryptor;
        std::vector<uint8_t> encryptedData;
        if (!encryptor.Encrypt(rawData.data(), rawData.size(), encryptedData))
        {
            std::cout << "[AssetEncryptor] Failed to encrypt data." << std::endl;
            return false;
        }

        AssetFileHeader header{};
        header.magic = ASSET_MAGIC_ENCRYPTED;
        header.version = 1;
        header.cipherType = static_cast<uint16_t>(AssetCipherType::Xor);
        header.originalSize = rawData.size();
        header.encryptedSize = encryptedData.size();
        header.checksum = ComputeChecksum(rawData.data(), rawData.size());

        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile.is_open())
        {
            std::cout << "[AssetEncryptor] Failed to open output file: " << outputPath << std::endl;
            return false;
        }

        outFile.write(reinterpret_cast<const char*>(&header), sizeof(header));
        outFile.write(reinterpret_cast<const char*>(encryptedData.data()), encryptedData.size());

        std::cout << "[AssetEncryptor] Encrypted: " << inputPath << " -> " << outputPath
                  << " (" << rawData.size() << " -> " << (sizeof(header) + encryptedData.size()) << " bytes)" << std::endl;
        return true;
    }

    bool DecryptModelFile(const std::string& inputPath, const std::string& outputPath)
    {
        std::ifstream inFile(inputPath, std::ios::binary | std::ios::ate);
        if (!inFile.is_open())
        {
            std::cout << "[AssetEncryptor] Failed to open input file: " << inputPath << std::endl;
            return false;
        }

        std::streamsize fileSize = inFile.tellg();
        inFile.seekg(0, std::ios::beg);

        if (fileSize < static_cast<std::streamsize>(sizeof(AssetFileHeader)))
        {
            std::cout << "[AssetEncryptor] File too small for header: " << inputPath << std::endl;
            return false;
        }

        AssetFileHeader header{};
        inFile.read(reinterpret_cast<char*>(&header), sizeof(header));
        if (!header.IsValid())
        {
            std::cout << "[AssetEncryptor] Invalid header magic: " << inputPath << std::endl;
            return false;
        }

        std::vector<uint8_t> encryptedData(static_cast<size_t>(header.encryptedSize));
        inFile.read(reinterpret_cast<char*>(encryptedData.data()), header.encryptedSize);

        XorDecryptor decryptor;
        std::vector<uint8_t> decryptedData;
        if (!decryptor.Decrypt(encryptedData.data(), encryptedData.size(), decryptedData))
        {
            std::cout << "[AssetEncryptor] Failed to decrypt data: " << inputPath << std::endl;
            return false;
        }

        std::ofstream outFile(outputPath, std::ios::binary);
        if (!outFile.is_open())
        {
            std::cout << "[AssetEncryptor] Failed to open output file: " << outputPath << std::endl;
            return false;
        }

        outFile.write(reinterpret_cast<const char*>(decryptedData.data()), decryptedData.size());
        std::cout << "[AssetEncryptor] Decrypted: " << inputPath << " -> " << outputPath << std::endl;
        return true;
    }
}
