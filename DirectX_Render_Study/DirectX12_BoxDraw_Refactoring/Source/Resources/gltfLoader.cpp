#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tiny_gltf.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "gltfLoader.h"
#include "GltfModelLoader.h"
#include "ModelData.h"
#include "AssetHeader.h"
#include "XorDecryptor.h"

// tinygltf::Model から LoadedModelData への変換
static bool ParseTinyGltfModel(const tinygltf::Model& model, const std::string& assetPathOrName, LoadedModelData& loadedModelData)
{
    //----- メッシュデータの取得 -----
    for (const auto& mesh : model.meshes)
    {
        std::string meshName = mesh.name;
        int PrimitiveNum = 0;

        for (const auto& primitive : mesh.primitives)
        {
            std::string primitiveName;
            MeshData meshData;
            std::vector<MeshVertex> vertices;

            const auto& attributes = primitive.attributes;
            if (attributes.find("POSITION") == attributes.end())
            {
                continue;
            }

            const tinygltf::Accessor& posAccessor = model.accessors[attributes.at("POSITION")];
            size_t VertexCount = posAccessor.count;

            vertices.resize(VertexCount);

            // デフォルト初期化
            for (size_t i = 0; i < VertexCount; i++)
            {
                vertices[i].boneIndices[0] = 0;
                vertices[i].boneIndices[1] = 0;
                vertices[i].boneIndices[2] = 0;
                vertices[i].boneIndices[3] = 0;
                vertices[i].boneWeights[0] = 1.0f;
                vertices[i].boneWeights[1] = 0.0f;
                vertices[i].boneWeights[2] = 0.0f;
                vertices[i].boneWeights[3] = 0.0f;
            }

            // POSITION
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("POSITION")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* pos = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].position[0] = pos[0];
                    vertices[i].position[1] = pos[1];
                    vertices[i].position[2] = pos[2];
                }
            }

            // NORMAL
            if (attributes.find("NORMAL") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* norm = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].normal[0] = norm[0];
                    vertices[i].normal[1] = norm[1];
                    vertices[i].normal[2] = norm[2];
                }
            }

            // TEXCOORD_0
            if (attributes.find("TEXCOORD_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* uv = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].uv[0] = uv[0];
                    vertices[i].uv[1] = uv[1];
                }
            }

            // JOINTS_0
            if (attributes.find("JOINTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("JOINTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint16_t* joints = reinterpret_cast<const uint16_t*>(dataPtr + i * stride);
                        vertices[i].boneIndices[0] = static_cast<uint32_t>(joints[0]);
                        vertices[i].boneIndices[1] = static_cast<uint32_t>(joints[1]);
                        vertices[i].boneIndices[2] = static_cast<uint32_t>(joints[2]);
                        vertices[i].boneIndices[3] = static_cast<uint32_t>(joints[3]);
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint8_t* joints = reinterpret_cast<const uint8_t*>(dataPtr + i * stride);
                        vertices[i].boneIndices[0] = static_cast<uint32_t>(joints[0]);
                        vertices[i].boneIndices[1] = static_cast<uint32_t>(joints[1]);
                        vertices[i].boneIndices[2] = static_cast<uint32_t>(joints[2]);
                        vertices[i].boneIndices[3] = static_cast<uint32_t>(joints[3]);
                    }
                }
            }

            // WEIGHTS_0
            if (attributes.find("WEIGHTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("WEIGHTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT)
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const float* weights = reinterpret_cast<const float*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = weights[0];
                        vertices[i].boneWeights[1] = weights[1];
                        vertices[i].boneWeights[2] = weights[2];
                        vertices[i].boneWeights[3] = weights[3];
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint16_t* weights = reinterpret_cast<const uint16_t*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = static_cast<float>(weights[0]) / 65535.0f;
                        vertices[i].boneWeights[1] = static_cast<float>(weights[1]) / 65535.0f;
                        vertices[i].boneWeights[2] = static_cast<float>(weights[2]) / 65535.0f;
                        vertices[i].boneWeights[3] = static_cast<float>(weights[3]) / 65535.0f;
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint8_t* weights = reinterpret_cast<const uint8_t*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = static_cast<float>(weights[0]) / 255.0f;
                        vertices[i].boneWeights[1] = static_cast<float>(weights[1]) / 255.0f;
                        vertices[i].boneWeights[2] = static_cast<float>(weights[2]) / 255.0f;
                        vertices[i].boneWeights[3] = static_cast<float>(weights[3]) / 255.0f;
                    }
                }
            }

            int materialIndex = primitive.material;
            meshData.materialIndex = materialIndex;

            primitiveName = meshName + "_Primitive" + std::to_string(PrimitiveNum);
            meshData.name = primitiveName;
            meshData.vertices = vertices;

            // Indices
            std::vector<uint32_t> indices;
            if (primitive.indices >= 0)
            {
                const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

                switch (accessor.componentType)
                {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                {
                    const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                    for (size_t i = 0; i < accessor.count; i++) {
                        indices.push_back(static_cast<uint32_t>(buf[i]));
                    }
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                {
                    const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                    for (size_t i = 0; i < accessor.count; i++) {
                        indices.push_back(buf[i]);
                    }
                    break;
                }
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                {
                    const uint8_t* buf = reinterpret_cast<const uint8_t*>(dataPtr);
                    for (size_t i = 0; i < accessor.count; i++) {
                        indices.push_back(static_cast<uint32_t>(buf[i]));
                    }
                    break;
                }
                default:
                    break;
                }
            }

            meshData.indices.assign(std::begin(indices), std::end(indices));
            loadedModelData.meshes.push_back(meshData);
            PrimitiveNum++;
        }
    }

    //----- マテリアルデータ取得 -----
    for (const auto& material : model.materials)
    {
        MaterialData materialData;
        materialData.name = material.name;

        const auto& pbr = material.pbrMetallicRoughness;

        if (!pbr.baseColorFactor.empty())
        {
            materialData.baseColorFactor[0] = static_cast<float>(pbr.baseColorFactor[0]);
            materialData.baseColorFactor[1] = static_cast<float>(pbr.baseColorFactor[1]);
            materialData.baseColorFactor[2] = static_cast<float>(pbr.baseColorFactor[2]);
            materialData.baseColorFactor[3] = static_cast<float>(pbr.baseColorFactor[3]);
        }

        materialData.metallicFactor = static_cast<float>(pbr.metallicFactor);
        materialData.roughnessFactor = static_cast<float>(pbr.roughnessFactor);

        // BaseColorTexture
        if (pbr.baseColorTexture.index >= 0)
        {
            int texIndex = pbr.baseColorTexture.index;
            const tinygltf::Texture& tex = model.textures[texIndex];

            if (tex.source >= 0)
            {
                const tinygltf::Image& image = model.images[tex.source];
                if (!image.uri.empty())
                {
                    materialData.baseColorTexturePath = image.uri;
                }
                else if (!image.image.empty())
                {
                    std::string baseName = "embedded";
                    size_t lastSlash = assetPathOrName.find_last_of("/\\");
                    if (lastSlash != std::string::npos)
                    {
                        baseName = assetPathOrName.substr(lastSlash + 1);
                        size_t lastDot = baseName.find_last_of(".");
                        if (lastDot != std::string::npos)
                        {
                            baseName = baseName.substr(0, lastDot);
                        }
                    }
                    std::string outPath = "Assets/Texture/" + baseName + "_tex_" + std::to_string(tex.source) + ".png";
                    if (stbi_write_png(outPath.c_str(), image.width, image.height, image.component, image.image.data(), image.width * image.component))
                    {
                        materialData.baseColorTexturePath = outPath;
                    }
                }
            }
        }

        // MetallicRoughnessTexture
        if (pbr.metallicRoughnessTexture.index >= 0)
        {
            int texIndex = pbr.metallicRoughnessTexture.index;
            const tinygltf::Texture& tex = model.textures[texIndex];

            if (tex.source >= 0)
            {
                const tinygltf::Image& image = model.images[tex.source];
                materialData.metallicRoughnessTexturePath = image.uri;
            }
        }

        loadedModelData.materials.push_back(materialData);
    }

    //----- ノードデータ取得 -----
    for (const auto& node : model.nodes)
    {
        NodeData nodeData;
        nodeData.name = node.name;
        nodeData.meshIndex = node.mesh;
        nodeData.children = node.children;

        if (!node.translation.empty()) {
            nodeData.translation[0] = static_cast<float>(node.translation[0]);
            nodeData.translation[1] = static_cast<float>(node.translation[1]);
            nodeData.translation[2] = static_cast<float>(node.translation[2]);
        }

        if (!node.rotation.empty()) {
            nodeData.rotation[0] = static_cast<float>(node.rotation[0]);
            nodeData.rotation[1] = static_cast<float>(node.rotation[1]);
            nodeData.rotation[2] = static_cast<float>(node.rotation[2]);
            nodeData.rotation[3] = static_cast<float>(node.rotation[3]);
        }

        if (!node.scale.empty()) {
            nodeData.scale[0] = static_cast<float>(node.scale[0]);
            nodeData.scale[1] = static_cast<float>(node.scale[1]);
            nodeData.scale[2] = static_cast<float>(node.scale[2]);
        }

        if (!node.matrix.empty())
        {
            for (int i = 0; i < 16; i++) {
                nodeData.matrix[i] = static_cast<float>(node.matrix[i]);
            }
        }

        nodeData.skinIndex = node.skin;
        loadedModelData.nodes.push_back(nodeData);
    }

    //----- スキンデータ取得 -----
    for (const auto& skin : model.skins)
    {
        SkinData skinData;
        skinData.joints = skin.joints;

        if (skin.inverseBindMatrices >= 0) {
            const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
            const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

            size_t count = accessor.count;
            skinData.inverseBindMatrices.resize(count);

            for (size_t i = 0; i < count; ++i) {
                const float* m = reinterpret_cast<const float*>(dataPtr + accessor.ByteStride(bufferView) * i);
                DirectX::XMFLOAT4X4 mat;
                memcpy(&mat, m, sizeof(float) * 16);
                skinData.inverseBindMatrices[i] = mat;
            }
        }

        loadedModelData.skins.push_back(skinData);
    }

    //----- アニメーション取得 -----
    for (const auto& anim : model.animations)
    {
        AnimationData animData;
        animData.name = anim.name;

        for (const auto& sampler : anim.samplers)
        {
            AnimationSamplerData samplerData;

            const tinygltf::Accessor& inputAccessor = model.accessors[sampler.input];
            const tinygltf::BufferView& inputView = model.bufferViews[inputAccessor.bufferView];
            const tinygltf::Buffer& inputBuffer = model.buffers[inputView.buffer];
            const float* times = reinterpret_cast<const float*>(&inputBuffer.data[inputView.byteOffset + inputAccessor.byteOffset]);
            for (size_t i = 0; i < inputAccessor.count; ++i) {
                samplerData.input.push_back(times[i]);
            }

            const tinygltf::Accessor& outputAccessor = model.accessors[sampler.output];
            const tinygltf::BufferView& outputView = model.bufferViews[outputAccessor.bufferView];
            const tinygltf::Buffer& outputBuffer = model.buffers[outputView.buffer];
            const unsigned char* outputDataPtr = &outputBuffer.data[outputView.byteOffset + outputAccessor.byteOffset];
            int numComponents = tinygltf::GetNumComponentsInType(outputAccessor.type);
            size_t stride = outputAccessor.ByteStride(outputView);

            for (size_t i = 0; i < outputAccessor.count; ++i)
            {
                std::vector<float> val(numComponents);
                const unsigned char* currentData = outputDataPtr + i * stride;

                for (int j = 0; j < numComponents; ++j) {
                    if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        val[j] = reinterpret_cast<const float*>(currentData)[j];
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
                        val[j] = std::max(reinterpret_cast<const int8_t*>(currentData)[j] / 127.0f, -1.0f);
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        val[j] = reinterpret_cast<const uint8_t*>(currentData)[j] / 255.0f;
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
                        val[j] = std::max(reinterpret_cast<const int16_t*>(currentData)[j] / 32767.0f, -1.0f);
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        val[j] = reinterpret_cast<const uint16_t*>(currentData)[j] / 65535.0f;
                    }
                }
                samplerData.output.push_back(val);
            }

            if (sampler.interpolation == "LINEAR") samplerData.interpolation = InterpolationType::LINEAR;
            else if (sampler.interpolation == "STEP") samplerData.interpolation = InterpolationType::STEP;
            else samplerData.interpolation = InterpolationType::CUBICSPLINE;

            animData.samplers.push_back(samplerData);
        }

        for (const auto& channel : anim.channels)
        {
            AnimationChannelData channelData;
            channelData.targetNodeIndex = channel.target_node;
            channelData.samplerIndex = channel.sampler;

            if (channel.target_path == "translation") channelData.path = AnimationPath::TRANSLATION;
            else if (channel.target_path == "rotation") channelData.path = AnimationPath::ROTATION;
            else if (channel.target_path == "scale") channelData.path = AnimationPath::SCALE;
            else channelData.path = AnimationPath::WEIGHTS;

            animData.channels.push_back(channelData);
        }

        loadedModelData.animations.push_back(animData);
    }

    return true;
}

// GltfModelLoader::LoadFromMemory の実装
bool GltfModelLoader::LoadFromMemory(const uint8_t* data, size_t size, const std::string& assetPathOrName, LoadedModelData& outData)
{
    if (!data || size == 0) return false;

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;

    std::string baseDir = "";
    size_t lastSlash = assetPathOrName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        baseDir = assetPathOrName.substr(0, lastSlash);
    }

    bool loadResult = false;
    // バイナリGLBかテキストGLTFかを判定
    if (size >= 4 && data[0] == 'g' && data[1] == 'l' && data[2] == 'T' && data[3] == 'F') {
        loadResult = loader.LoadBinaryFromMemory(&model, &err, &warn, data, static_cast<unsigned int>(size), baseDir);
    } else {
        loadResult = loader.LoadASCIIFromString(&model, &err, &warn, reinterpret_cast<const char*>(data), static_cast<unsigned int>(size), baseDir);
    }

    if (!warn.empty()) {
        std::cout << "[GltfModelLoader] Warn: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cout << "[GltfModelLoader] Err: " << err << std::endl;
    }
    if (!loadResult) {
        std::cout << "[GltfModelLoader] Failed to load model from memory: " << assetPathOrName << std::endl;
        return false;
    }

    return ParseTinyGltfModel(model, assetPathOrName, outData);
}

// ファイル読み込み ＆ 暗号化自動検知＆復号
LoadedModelData LoadModelDataFromFile(const std::string& filePath)
{
    LoadedModelData loadedModelData;

    // パス解決：もし要求されたファイルが存在しない場合のフォールバック
    std::string resolvedPath = filePath;
    if (!std::filesystem::exists(resolvedPath))
    {
        // .glb -> .dat
        if (resolvedPath.ends_with(".glb"))
        {
            std::string datPath = resolvedPath.substr(0, resolvedPath.length() - 4) + ".dat";
            if (std::filesystem::exists(datPath))
            {
                resolvedPath = datPath;
            }
        }
        // .dat -> .glb
        else if (resolvedPath.ends_with(".dat"))
        {
            std::string glbPath = resolvedPath.substr(0, resolvedPath.length() - 4) + ".glb";
            if (std::filesystem::exists(glbPath))
            {
                resolvedPath = glbPath;
            }
        }
    }

    std::ifstream file(resolvedPath, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cout << "[LoadModelDataFromFile] Failed to open: " << resolvedPath << std::endl;
        return loadedModelData;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(buffer.data()), fileSize))
    {
        std::cout << "[LoadModelDataFromFile] Failed to read: " << resolvedPath << std::endl;
        return loadedModelData;
    }

    // 暗号化ヘッダーの自動判定
    std::vector<uint8_t> decryptedBuffer;
    const uint8_t* parseData = buffer.data();
    size_t parseSize = buffer.size();

    if (buffer.size() >= sizeof(AssetSecurity::AssetFileHeader))
    {
        const auto* header = reinterpret_cast<const AssetSecurity::AssetFileHeader*>(buffer.data());
        if (header->IsValid())
        {
            const uint8_t* encryptedData = buffer.data() + sizeof(AssetSecurity::AssetFileHeader);
            size_t encryptedSize = buffer.size() - sizeof(AssetSecurity::AssetFileHeader);

            std::unique_ptr<IAssetDecryptor> decryptor;
            if (header->cipherType == static_cast<uint16_t>(AssetSecurity::AssetCipherType::Xor))
            {
                decryptor = std::make_unique<XorDecryptor>();
            }
            else
            {
                decryptor = std::make_unique<RawDecryptor>();
            }

            if (decryptor->Decrypt(encryptedData, encryptedSize, decryptedBuffer))
            {
                // チェックサム検証
                if (header->checksum != 0)
                {
                    uint32_t calcCrc = AssetSecurity::ComputeChecksum(decryptedBuffer.data(), decryptedBuffer.size());
                    if (calcCrc != header->checksum)
                    {
                        std::cout << "[LoadModelDataFromFile] Checksum mismatch for " << resolvedPath << std::endl;
                    }
                }

                parseData = decryptedBuffer.data();
                parseSize = decryptedBuffer.size();
                std::cout << "[LoadModelDataFromFile] Successfully decrypted encrypted asset: " << resolvedPath << std::endl;
            }
            else
            {
                std::cout << "[LoadModelDataFromFile] Failed to decrypt asset: " << resolvedPath << std::endl;
                return loadedModelData;
            }
        }
    }

    GltfModelLoader loader;
    loader.LoadFromMemory(parseData, parseSize, resolvedPath, loadedModelData);
    return loadedModelData;
}

// 互換性維持のための関数
LoadedModelData TestLoadGLTF(std::string _fileName)
{
    return LoadModelDataFromFile(_fileName);
}
