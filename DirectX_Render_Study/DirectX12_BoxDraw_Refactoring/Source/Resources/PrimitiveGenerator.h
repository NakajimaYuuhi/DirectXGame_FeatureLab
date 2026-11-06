#pragma once
#include <vector>
#include <memory>
#include <string>
#include <DirectXMath.h>
#include "ModelData.h"

class CMesh;
class CModel;
class CObject;

/**
 * @brief 3D Primitive mesh and model generation utility class
 */
class PrimitiveGenerator
{
public:
    // ==========================================
    // Primitive MeshData Generation
    // ==========================================

    /**
     * @brief Create Cube / Box mesh
     */
    static MeshData CreateCube(float width = 1.0f, float height = 1.0f, float depth = 1.0f);

    /**
     * @brief Create Plane / Grid mesh (XZ plane, Y=0, Normal facing up)
     */
    static MeshData CreatePlane(float width = 1.0f, float depth = 1.0f, uint32_t subdivisionsX = 1, uint32_t subdivisionsZ = 1);

    /**
     * @brief Create Circle / Disc mesh (XZ plane, Y=0, Normal facing up)
     */
    static MeshData CreateCircle(float radius = 0.5f, uint32_t sliceCount = 32);

    /**
     * @brief Create Sphere mesh (UV Sphere)
     */
    static MeshData CreateSphere(float radius = 0.5f, uint32_t sliceCount = 32, uint32_t stackCount = 16);

    /**
     * @brief Create Cylinder mesh
     */
    static MeshData CreateCylinder(float bottomRadius = 0.5f, float topRadius = 0.5f, float height = 1.0f, uint32_t sliceCount = 32, uint32_t stackCount = 1);

    /**
     * @brief Create Cone mesh
     */
    static MeshData CreateCone(float radius = 0.5f, float height = 1.0f, uint32_t sliceCount = 32);

    /**
     * @brief Create Capsule mesh (Hemispheres + Cylinder body)
     */
    static MeshData CreateCapsule(float radius = 0.5f, float cylinderHeight = 1.0f, uint32_t sliceCount = 32, uint32_t stackCount = 8);

    /**
     * @brief Create Torus / Donut mesh
     */
    static MeshData CreateTorus(float majorRadius = 1.0f, float minorRadius = 0.25f, uint32_t majorSegments = 32, uint32_t minorSegments = 16);

    // ==========================================
    // Instance Helpers
    // ==========================================

    /**
     * @brief Create CMesh from MeshData
     */
    static std::shared_ptr<CMesh> CreateMesh(const MeshData& meshData, CObject* owner = nullptr);

    /**
     * @brief Create CModel from MeshData
     */
    static std::shared_ptr<CModel> CreateModel(const MeshData& meshData, const std::wstring& texturePath = L"", const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    /**
     * @brief Create CModel by primitive type name
     */
    static std::shared_ptr<CModel> CreatePrimitiveModel(const std::string& primitiveType, const std::wstring& texturePath = L"", const DirectX::XMFLOAT4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

    /**
     * @brief Check if string indicates a primitive specification
     */
    static bool IsPrimitivePath(const std::string& path);

    /**
     * @brief Extract primitive type from path string
     */
    static std::string ExtractPrimitiveType(const std::string& path);
};
