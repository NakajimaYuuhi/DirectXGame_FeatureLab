#include "PrimitiveGenerator.h"
#include "Mesh.h"
#include "Model.h"
#include "Material.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = PI * 2.0f;
    constexpr float HALF_PI = PI * 0.5f;

    MeshVertex MakeVertex(float px, float py, float pz,
                          float nx, float ny, float nz,
                          float u, float v)
    {
        MeshVertex vert;
        vert.position[0] = px;
        vert.position[1] = py;
        vert.position[2] = pz;

        vert.normal[0] = nx;
        vert.normal[1] = ny;
        vert.normal[2] = nz;

        vert.uv[0] = u;
        vert.uv[1] = v;

        vert.boneIndices[0] = 0;
        vert.boneIndices[1] = 0;
        vert.boneIndices[2] = 0;
        vert.boneIndices[3] = 0;

        vert.boneWeights[0] = 1.0f;
        vert.boneWeights[1] = 0.0f;
        vert.boneWeights[2] = 0.0f;
        vert.boneWeights[3] = 0.0f;

        return vert;
    }
}

// ==========================================
// Cube / Box
// ==========================================
MeshData PrimitiveGenerator::CreateCube(float width, float height, float depth)
{
    MeshData mesh;
    mesh.name = "Primitive_Cube";
    mesh.materialIndex = 0;

    float w = width * 0.5f;
    float h = height * 0.5f;
    float d = depth * 0.5f;

    mesh.vertices.reserve(24);
    mesh.indices.reserve(36);

    auto addQuad = [&](const MeshVertex& v0, const MeshVertex& v1, const MeshVertex& v2, const MeshVertex& v3)
    {
        uint32_t startIndex = static_cast<uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back(v0);
        mesh.vertices.push_back(v1);
        mesh.vertices.push_back(v2);
        mesh.vertices.push_back(v3);

        // Clockwise (CW)
        mesh.indices.push_back(startIndex + 0);
        mesh.indices.push_back(startIndex + 1);
        mesh.indices.push_back(startIndex + 2);

        mesh.indices.push_back(startIndex + 0);
        mesh.indices.push_back(startIndex + 2);
        mesh.indices.push_back(startIndex + 3);
    };

    // Front (+Z)
    addQuad(
        MakeVertex(-w,  h,  d,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f),
        MakeVertex( w,  h,  d,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f),
        MakeVertex( w, -h,  d,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f),
        MakeVertex(-w, -h,  d,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f)
    );

    // Back (-Z)
    addQuad(
        MakeVertex( w,  h, -d,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f),
        MakeVertex(-w,  h, -d,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f),
        MakeVertex(-w, -h, -d,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f),
        MakeVertex( w, -h, -d,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f)
    );

    // Top (+Y)
    addQuad(
        MakeVertex(-w,  h, -d,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f),
        MakeVertex( w,  h, -d,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f),
        MakeVertex( w,  h,  d,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f),
        MakeVertex(-w,  h,  d,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f)
    );

    // Bottom (-Y)
    addQuad(
        MakeVertex(-w, -h,  d,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f),
        MakeVertex( w, -h,  d,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f),
        MakeVertex( w, -h, -d,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f),
        MakeVertex(-w, -h, -d,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f)
    );

    // Left (-X)
    addQuad(
        MakeVertex(-w,  h, -d, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f),
        MakeVertex(-w,  h,  d, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f),
        MakeVertex(-w, -h,  d, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f),
        MakeVertex(-w, -h, -d, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f)
    );

    // Right (+X)
    addQuad(
        MakeVertex( w,  h,  d,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f),
        MakeVertex( w,  h, -d,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f),
        MakeVertex( w, -h, -d,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f),
        MakeVertex( w, -h,  d,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f)
    );

    return mesh;
}

// ==========================================
// Plane / Grid
// ==========================================
MeshData PrimitiveGenerator::CreatePlane(float width, float depth, uint32_t subdivisionsX, uint32_t subdivisionsZ)
{
    MeshData mesh;
    mesh.name = "Primitive_Plane";
    mesh.materialIndex = 0;

    subdivisionsX = (std::max)(1u, subdivisionsX);
    subdivisionsZ = (std::max)(1u, subdivisionsZ);

    uint32_t vertCountX = subdivisionsX + 1;
    uint32_t vertCountZ = subdivisionsZ + 1;

    mesh.vertices.reserve(vertCountX * vertCountZ);
    mesh.indices.reserve(subdivisionsX * subdivisionsZ * 6);

    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;

    float dx = width / static_cast<float>(subdivisionsX);
    float dz = depth / static_cast<float>(subdivisionsZ);

    for (uint32_t z = 0; z < vertCountZ; ++z)
    {
        float pz = -halfD + static_cast<float>(z) * dz;
        float v = static_cast<float>(z) / static_cast<float>(subdivisionsZ);

        for (uint32_t x = 0; x < vertCountX; ++x)
        {
            float px = -halfW + static_cast<float>(x) * dx;
            float u = static_cast<float>(x) / static_cast<float>(subdivisionsX);

            mesh.vertices.push_back(MakeVertex(px, 0.0f, pz, 0.0f, 1.0f, 0.0f, u, v));
        }
    }

    for (uint32_t z = 0; z < subdivisionsZ; ++z)
    {
        for (uint32_t x = 0; x < subdivisionsX; ++x)
        {
            uint32_t row1 = z * vertCountX;
            uint32_t row2 = (z + 1) * vertCountX;

            uint32_t v0 = row1 + x;
            uint32_t v1 = row1 + x + 1;
            uint32_t v2 = row2 + x;
            uint32_t v3 = row2 + x + 1;

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v1);
            mesh.indices.push_back(v3);

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v3);
            mesh.indices.push_back(v2);
        }
    }

    return mesh;
}

// ==========================================
// Circle / Disc
// ==========================================
MeshData PrimitiveGenerator::CreateCircle(float radius, uint32_t sliceCount)
{
    MeshData mesh;
    mesh.name = "Primitive_Circle";
    mesh.materialIndex = 0;

    sliceCount = (std::max)(3u, sliceCount);

    mesh.vertices.reserve(sliceCount + 2);
    mesh.indices.reserve(sliceCount * 3);

    // Center vertex
    mesh.vertices.push_back(MakeVertex(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f));

    float deltaTheta = TWO_PI / static_cast<float>(sliceCount);

    for (uint32_t i = 0; i <= sliceCount; ++i)
    {
        float theta = static_cast<float>(i) * deltaTheta;
        float x = radius * std::cos(theta);
        float z = radius * std::sin(theta);

        float u = (x / radius) * 0.5f + 0.5f;
        float v = (z / radius) * 0.5f + 0.5f;

        mesh.vertices.push_back(MakeVertex(x, 0.0f, z, 0.0f, 1.0f, 0.0f, u, v));
    }

    for (uint32_t i = 1; i <= sliceCount; ++i)
    {
        mesh.indices.push_back(0);
        mesh.indices.push_back(i);
        mesh.indices.push_back(i + 1);
    }

    return mesh;
}

// ==========================================
// Sphere (UV Sphere)
// ==========================================
MeshData PrimitiveGenerator::CreateSphere(float radius, uint32_t sliceCount, uint32_t stackCount)
{
    MeshData mesh;
    mesh.name = "Primitive_Sphere";
    mesh.materialIndex = 0;

    sliceCount = (std::max)(3u, sliceCount);
    stackCount = (std::max)(2u, stackCount);

    uint32_t vertCount = (stackCount + 1) * (sliceCount + 1);
    mesh.vertices.reserve(vertCount);
    mesh.indices.reserve(stackCount * sliceCount * 6);

    float deltaPhi = PI / static_cast<float>(stackCount);
    float deltaTheta = TWO_PI / static_cast<float>(sliceCount);

    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        float phi = static_cast<float>(i) * deltaPhi;
        float y = radius * std::cos(phi);
        float rSinPhi = radius * std::sin(phi);
        float v = static_cast<float>(i) / static_cast<float>(stackCount);

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float x = rSinPhi * std::sin(theta);
            float z = rSinPhi * std::cos(theta);
            float u = static_cast<float>(j) / static_cast<float>(sliceCount);

            float nx = x / radius;
            float ny = y / radius;
            float nz = z / radius;

            mesh.vertices.push_back(MakeVertex(x, y, z, nx, ny, nz, u, v));
        }
    }

    uint32_t stride = sliceCount + 1;
    for (uint32_t i = 0; i < stackCount; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            uint32_t v0 = i * stride + j;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = (i + 1) * stride + j;
            uint32_t v3 = v2 + 1;

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v1);
            mesh.indices.push_back(v3);

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v3);
            mesh.indices.push_back(v2);
        }
    }

    return mesh;
}

// ==========================================
// Cylinder
// ==========================================
MeshData PrimitiveGenerator::CreateCylinder(float bottomRadius, float topRadius, float height, uint32_t sliceCount, uint32_t stackCount)
{
    MeshData mesh;
    mesh.name = "Primitive_Cylinder";
    mesh.materialIndex = 0;

    sliceCount = (std::max)(3u, sliceCount);
    stackCount = (std::max)(1u, stackCount);

    float stackHeight = height / static_cast<float>(stackCount);
    float radiusStep = (topRadius - bottomRadius) / static_cast<float>(stackCount);
    float deltaTheta = TWO_PI / static_cast<float>(sliceCount);

    uint32_t ringCount = stackCount + 1;
    uint32_t stride = sliceCount + 1;

    // Side vertices
    for (uint32_t i = 0; i < ringCount; ++i)
    {
        float y = -0.5f * height + static_cast<float>(i) * stackHeight;
        float r = bottomRadius + static_cast<float>(i) * radiusStep;
        float v = 1.0f - static_cast<float>(i) / static_cast<float>(stackCount);

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float cosT = std::cos(theta);
            float sinT = std::sin(theta);

            float x = r * sinT;
            float z = r * cosT;
            float u = static_cast<float>(j) / static_cast<float>(sliceCount);

            float dr = bottomRadius - topRadius;
            DirectX::XMFLOAT3 normalVal = { sinT, dr / height, cosT };
            DirectX::XMVECTOR nVec = DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&normalVal));
            DirectX::XMStoreFloat3(&normalVal, nVec);

            mesh.vertices.push_back(MakeVertex(x, y, z, normalVal.x, normalVal.y, normalVal.z, u, v));
        }
    }

    // Side indices
    for (uint32_t i = 0; i < stackCount; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            uint32_t v0 = i * stride + j;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = (i + 1) * stride + j;
            uint32_t v3 = v2 + 1;

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v2);
            mesh.indices.push_back(v3);

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v3);
            mesh.indices.push_back(v1);
        }
    }

    // Top Cap
    if (topRadius > 0.0f)
    {
        uint32_t topCenterIndex = static_cast<uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back(MakeVertex(0.0f, 0.5f * height, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f));

        uint32_t topRingStart = static_cast<uint32_t>(mesh.vertices.size());
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float x = topRadius * std::sin(theta);
            float z = topRadius * std::cos(theta);
            float u = (x / topRadius) * 0.5f + 0.5f;
            float v = (z / topRadius) * 0.5f + 0.5f;

            mesh.vertices.push_back(MakeVertex(x, 0.5f * height, z, 0.0f, 1.0f, 0.0f, u, v));
        }

        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            mesh.indices.push_back(topCenterIndex);
            mesh.indices.push_back(topRingStart + j + 1);
            mesh.indices.push_back(topRingStart + j);
        }
    }

    // Bottom Cap
    if (bottomRadius > 0.0f)
    {
        uint32_t bottomCenterIndex = static_cast<uint32_t>(mesh.vertices.size());
        mesh.vertices.push_back(MakeVertex(0.0f, -0.5f * height, 0.0f, 0.0f, -1.0f, 0.0f, 0.5f, 0.5f));

        uint32_t bottomRingStart = static_cast<uint32_t>(mesh.vertices.size());
        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float x = bottomRadius * std::sin(theta);
            float z = bottomRadius * std::cos(theta);
            float u = (x / bottomRadius) * 0.5f + 0.5f;
            float v = (z / bottomRadius) * 0.5f + 0.5f;

            mesh.vertices.push_back(MakeVertex(x, -0.5f * height, z, 0.0f, -1.0f, 0.0f, u, v));
        }

        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            mesh.indices.push_back(bottomCenterIndex);
            mesh.indices.push_back(bottomRingStart + j);
            mesh.indices.push_back(bottomRingStart + j + 1);
        }
    }

    return mesh;
}

// ==========================================
// Cone
// ==========================================
MeshData PrimitiveGenerator::CreateCone(float radius, float height, uint32_t sliceCount)
{
    MeshData mesh = CreateCylinder(radius, 0.0f, height, sliceCount, 1);
    mesh.name = "Primitive_Cone";
    return mesh;
}

// ==========================================
// Capsule
// ==========================================
MeshData PrimitiveGenerator::CreateCapsule(float radius, float cylinderHeight, uint32_t sliceCount, uint32_t stackCount)
{
    MeshData mesh;
    mesh.name = "Primitive_Capsule";
    mesh.materialIndex = 0;

    sliceCount = (std::max)(3u, sliceCount);
    stackCount = (std::max)(1u, stackCount);

    float halfHeight = cylinderHeight * 0.5f;
    float deltaTheta = TWO_PI / static_cast<float>(sliceCount);
    float deltaPhi = HALF_PI / static_cast<float>(stackCount);

    // 1. Top Hemisphere
    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        float phi = static_cast<float>(i) * deltaPhi;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        float y = halfHeight + radius * cosPhi;
        float r = radius * sinPhi;
        float v = (static_cast<float>(i) / static_cast<float>(stackCount * 2 + 1)) * 0.5f;

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float sinT = std::sin(theta);
            float cosT = std::cos(theta);

            float x = r * sinT;
            float z = r * cosT;
            float u = static_cast<float>(j) / static_cast<float>(sliceCount);

            float nx = sinPhi * sinT;
            float ny = cosPhi;
            float nz = sinPhi * cosT;

            mesh.vertices.push_back(MakeVertex(x, y, z, nx, ny, nz, u, v));
        }
    }

    // 2. Bottom Hemisphere
    for (uint32_t i = 0; i <= stackCount; ++i)
    {
        float phi = HALF_PI + static_cast<float>(i) * deltaPhi;
        float cosPhi = std::cos(phi);
        float sinPhi = std::sin(phi);

        float y = -halfHeight + radius * cosPhi;
        float r = radius * sinPhi;
        float v = 0.5f + (static_cast<float>(i) / static_cast<float>(stackCount)) * 0.5f;

        for (uint32_t j = 0; j <= sliceCount; ++j)
        {
            float theta = static_cast<float>(j) * deltaTheta;
            float sinT = std::sin(theta);
            float cosT = std::cos(theta);

            float x = r * sinT;
            float z = r * cosT;
            float u = static_cast<float>(j) / static_cast<float>(sliceCount);

            float nx = sinPhi * sinT;
            float ny = cosPhi;
            float nz = sinPhi * cosT;

            mesh.vertices.push_back(MakeVertex(x, y, z, nx, ny, nz, u, v));
        }
    }

    uint32_t totalRings = (stackCount + 1) * 2;
    uint32_t stride = sliceCount + 1;

    for (uint32_t i = 0; i < totalRings - 1; ++i)
    {
        for (uint32_t j = 0; j < sliceCount; ++j)
        {
            uint32_t v0 = i * stride + j;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = (i + 1) * stride + j;
            uint32_t v3 = v2 + 1;

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v1);
            mesh.indices.push_back(v3);

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v3);
            mesh.indices.push_back(v2);
        }
    }

    return mesh;
}

// ==========================================
// Torus / Donut
// ==========================================
MeshData PrimitiveGenerator::CreateTorus(float majorRadius, float minorRadius, uint32_t majorSegments, uint32_t minorSegments)
{
    MeshData mesh;
    mesh.name = "Primitive_Torus";
    mesh.materialIndex = 0;

    majorSegments = (std::max)(3u, majorSegments);
    minorSegments = (std::max)(3u, minorSegments);

    float deltaMajor = TWO_PI / static_cast<float>(majorSegments);
    float deltaMinor = TWO_PI / static_cast<float>(minorSegments);

    for (uint32_t i = 0; i <= majorSegments; ++i)
    {
        float uAngle = static_cast<float>(i) * deltaMajor;
        float cosU = std::cos(uAngle);
        float sinU = std::sin(uAngle);
        float uCoord = static_cast<float>(i) / static_cast<float>(majorSegments);

        for (uint32_t j = 0; j <= minorSegments; ++j)
        {
            float vAngle = static_cast<float>(j) * deltaMinor;
            float cosV = std::cos(vAngle);
            float sinV = std::sin(vAngle);
            float vCoord = static_cast<float>(j) / static_cast<float>(minorSegments);

            float r = majorRadius + minorRadius * cosV;
            float x = r * sinU;
            float y = minorRadius * sinV;
            float z = r * cosU;

            float nx = cosV * sinU;
            float ny = sinV;
            float nz = cosV * cosU;

            mesh.vertices.push_back(MakeVertex(x, y, z, nx, ny, nz, uCoord, vCoord));
        }
    }

    uint32_t stride = minorSegments + 1;
    for (uint32_t i = 0; i < majorSegments; ++i)
    {
        for (uint32_t j = 0; j < minorSegments; ++j)
        {
            uint32_t v0 = i * stride + j;
            uint32_t v1 = v0 + 1;
            uint32_t v2 = (i + 1) * stride + j;
            uint32_t v3 = v2 + 1;

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v1);
            mesh.indices.push_back(v3);

            mesh.indices.push_back(v0);
            mesh.indices.push_back(v3);
            mesh.indices.push_back(v2);
        }
    }

    return mesh;
}

// ==========================================
// Instance Helpers
// ==========================================
std::shared_ptr<CMesh> PrimitiveGenerator::CreateMesh(const MeshData& meshData, CObject* owner)
{
    auto mesh = std::make_shared<CMesh>();
    if (owner)
    {
        mesh->RegisterOwner(owner);
    }
    mesh->Init();

    if (!meshData.vertices.empty() && !meshData.indices.empty())
    {
        mesh->SetVertex(meshData.vertices.data(), meshData.vertices.size(),
                        meshData.indices.data(), meshData.indices.size());
    }

    return mesh;
}

std::shared_ptr<CModel> PrimitiveGenerator::CreateModel(const MeshData& meshData, const std::wstring& texturePath, const DirectX::XMFLOAT4& color)
{
    auto model = std::make_shared<CModel>();

    UINT matIdx = model->RegisterMatarial(texturePath, color);

    if (!meshData.vertices.empty() && !meshData.indices.empty())
    {
        model->RegisterMesh(matIdx, meshData.vertices.data(), meshData.vertices.size(),
                            meshData.indices.data(), meshData.indices.size());
    }

    model->SetModelPath(meshData.name);
    return model;
}

std::shared_ptr<CModel> PrimitiveGenerator::CreatePrimitiveModel(const std::string& primitiveType, const std::wstring& texturePath, const DirectX::XMFLOAT4& color)
{
    std::string type = ExtractPrimitiveType(primitiveType);

    if (type == "cube" || type == "box")
    {
        return CreateModel(CreateCube(), texturePath, color);
    }
    else if (type == "plane" || type == "quad" || type == "grid")
    {
        return CreateModel(CreatePlane(), texturePath, color);
    }
    else if (type == "circle" || type == "disc")
    {
        return CreateModel(CreateCircle(), texturePath, color);
    }
    else if (type == "sphere")
    {
        return CreateModel(CreateSphere(), texturePath, color);
    }
    else if (type == "cylinder")
    {
        return CreateModel(CreateCylinder(), texturePath, color);
    }
    else if (type == "cone")
    {
        return CreateModel(CreateCone(), texturePath, color);
    }
    else if (type == "capsule")
    {
        return CreateModel(CreateCapsule(), texturePath, color);
    }
    else if (type == "torus" || type == "donut")
    {
        return CreateModel(CreateTorus(), texturePath, color);
    }

    return CreateModel(CreateCube(), texturePath, color);
}

bool PrimitiveGenerator::IsPrimitivePath(const std::string& path)
{
    if (path.rfind("primitive::", 0) == 0) return true;

    std::string lower = path;
    for (char& c : lower)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    return (lower == "cube" || lower == "box" ||
            lower == "sphere" ||
            lower == "cylinder" ||
            lower == "capsule" ||
            lower == "plane" || lower == "quad" ||
            lower == "circle" || lower == "disc" ||
            lower == "cone" ||
            lower == "torus" || lower == "donut");
}

std::string PrimitiveGenerator::ExtractPrimitiveType(const std::string& path)
{
    std::string lower = path;
    if (lower.rfind("primitive::", 0) == 0)
    {
        lower = lower.substr(11);
    }
    for (char& c : lower)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return lower;
}
