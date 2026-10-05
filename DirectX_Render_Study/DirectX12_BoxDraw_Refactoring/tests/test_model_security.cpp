#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <fstream>
#include "Source/Resources/AssetHeader.h"
#include "Source/Resources/XorDecryptor.h"
#include "Source/Resources/AssetEncryptor.h"
#include "Source/Resources/gltfLoader.h"

int main()
{
    std::cout << "=== Running Model Security & Encryption Tests ===" << std::endl;

    // Test 1: XorDecryptor In-Memory Encrypt / Decrypt
    {
        std::cout << "[Test 1] In-Memory XOR Encryption / Decryption..." << std::endl;
        std::string original = "The quick brown fox jumps over the lazy dog. 1234567890!@#$%^&*()";
        const uint8_t* rawData = reinterpret_cast<const uint8_t*>(original.data());
        size_t rawSize = original.size();

        XorDecryptor cryptor;
        std::vector<uint8_t> encrypted;
        bool encSuccess = cryptor.Encrypt(rawData, rawSize, encrypted);
        assert(encSuccess);
        assert(encrypted.size() == rawSize);

        // Verify it is actually scrambled
        bool isDifferent = false;
        for (size_t i = 0; i < rawSize; ++i) {
            if (encrypted[i] != rawData[i]) {
                isDifferent = true;
                break;
            }
        }
        assert(isDifferent);

        // Decrypt
        std::vector<uint8_t> decrypted;
        bool decSuccess = cryptor.Decrypt(encrypted.data(), encrypted.size(), decrypted);
        assert(decSuccess);
        assert(decrypted.size() == rawSize);

        std::string result(reinterpret_cast<const char*>(decrypted.data()), decrypted.size());
        assert(result == original);
        std::cout << "  -> PASSED: In-memory round-trip match!" << std::endl;
    }

    // Test 2: C++ File Encryption & Decryption Round-Trip
    {
        std::cout << "[Test 2] C++ File Encrypt / Decrypt Round-Trip..." << std::endl;
        std::string src = "Assets/Model/cube.glb";
        std::string enc = "Assets/Model/cube_test.dat";
        std::string dec = "Assets/Model/cube_test_dec.glb";

        bool encOk = AssetSecurity::EncryptModelFile(src, enc);
        assert(encOk);

        bool decOk = AssetSecurity::DecryptModelFile(enc, dec);
        assert(decOk);

        // Compare original and decrypted files byte-by-byte
        std::ifstream f1(src, std::ios::binary);
        std::ifstream f2(dec, std::ios::binary);
        std::vector<uint8_t> b1((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
        std::vector<uint8_t> b2((std::istreambuf_iterator<char>(f2)), std::istreambuf_iterator<char>());

        assert(b1.size() == b2.size());
        assert(b1 == b2);

        // Cleanup
        std::remove(enc.c_str());
        std::remove(dec.c_str());
        std::cout << "  -> PASSED: File round-trip binary match!" << std::endl;
    }

    // Test 3: LoadModelDataFromFile for both .glb and .dat
    {
        std::cout << "[Test 3] Model Loading Comparison (Plaintext .glb vs Encrypted .dat)..." << std::endl;
        
        // Ensure cube.dat exists
        AssetSecurity::EncryptModelFile("Assets/Model/cube.glb", "Assets/Model/cube.dat");

        LoadedModelData glbData = LoadModelDataFromFile("Assets/Model/cube.glb");
        LoadedModelData datData = LoadModelDataFromFile("Assets/Model/cube.dat");

        std::cout << "  .glb meshes: " << glbData.meshes.size() << ", .dat meshes: " << datData.meshes.size() << std::endl;
        assert(glbData.meshes.size() == datData.meshes.size());
        assert(!glbData.meshes.empty());

        assert(glbData.meshes[0].vertices.size() == datData.meshes[0].vertices.size());
        assert(glbData.meshes[0].indices.size() == datData.meshes[0].indices.size());
        std::cout << "  Vertex count: " << glbData.meshes[0].vertices.size()
                  << ", Index count: " << glbData.meshes[0].indices.size() << std::endl;

        // Verify vertex data matches
        for (size_t i = 0; i < glbData.meshes[0].vertices.size(); ++i)
        {
            assert(glbData.meshes[0].vertices[i].position[0] == datData.meshes[0].vertices[i].position[0]);
            assert(glbData.meshes[0].vertices[i].position[1] == datData.meshes[0].vertices[i].position[1]);
            assert(glbData.meshes[0].vertices[i].position[2] == datData.meshes[0].vertices[i].position[2]);
        }
        std::cout << "  -> PASSED: LoadedModelData from .glb and .dat are 100% identical!" << std::endl;
    }

    std::cout << "=== ALL TESTS PASSED SUCCESSFULLY! ===" << std::endl;
    return 0;
}
