#pragma once
#include <DirectXMath.h>
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
    //マテリアル名
    std::string name;

    //ベースカラーのテクスチャのパス
    std::string baseColorTexturePath;

    //ベースカラー
    float baseColorFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    //PBRの金属反射用
    float metallicFactor = 1.0f;
    float roughnessFactor = 1.0f;
    std::string metallicRoughnessTexturePath;
};

struct NodeData
{
    std::string name;
    int meshIndex = -1;

    std::vector<int> children;

    float translation[3] = { 0,0,0 };
    float rotation[4] = { 0,0,0,1 };
    float scale[3] = { 1,1,1 };
    float matrix[16] = {};  // デフォルトは identity 行列でも良い

    int skinIndex = -1;
};

struct LoadedModelData
{
    std::vector<MeshData> meshes;
    std::vector<MaterialData> materials;
    std::vector<NodeData> nodes;
};