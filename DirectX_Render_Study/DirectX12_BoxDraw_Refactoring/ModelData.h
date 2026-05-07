#pragma once

#include <vector>
#include <string>

//頂点情報
struct MeshVertex
{
    float position[3];
    float normal[3];
    float uv[2];

    uint8_t boneIndices[4];  // ボーン番号（最大255体まで）
    float   boneWeights[4];  // ボーンの重み
};

struct MeshData
{
    std::string name;

    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;

    uint32_t materialIndex;
};

struct MaterialData
{
    std::string name;

    std::string baseColorTexturePath;
};

struct NodeData
{
    std::string name;

    //Matrix localMatrix;

    std::vector<uint32_t> children;
};

struct LoadedModelData
{
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
    std::vector<NodeData> nodes;
};