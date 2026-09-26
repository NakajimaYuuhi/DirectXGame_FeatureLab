#pragma once
#include <DirectXMath.h>
#include <vector>
#include <memory>

class CObject;
class Camera;

struct Ray
{
    DirectX::XMFLOAT3 origin{ 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 direction{ 0.0f, 0.0f, 1.0f };
};

class EditorRaycast
{
public:
    static Ray CreateRayFromScreen(
        float screenX, float screenY,
        float screenWidth, float screenHeight,
        const DirectX::XMMATRIX& view,
        const DirectX::XMMATRIX& proj
    );

    static bool RayIntersectAABB(
        const Ray& ray,
        const DirectX::XMFLOAT3& boxMin,
        const DirectX::XMFLOAT3& boxMax,
        float& outDistance
    );

    static CObject* PickObject(
        float screenX, float screenY,
        float screenWidth, float screenHeight,
        Camera* camera,
        const std::vector<std::vector<std::unique_ptr<CObject>>>& objectList,
        int& outTagIndex,
        int& outObjectIndex
    );
};
