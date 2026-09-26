#include "EditorRaycast.h"
#include "Source/Objects/Camera.h"
#include "Object.h"
#include "Transform.h"
#include "BoxCollider3D.h"
#include "ObjectInfo.h"
#include <algorithm>
#include <cmath>

using namespace DirectX;

Ray EditorRaycast::CreateRayFromScreen(
    float screenX, float screenY,
    float screenWidth, float screenHeight,
    const XMMATRIX& view,
    const XMMATRIX& proj)
{
    if (screenWidth <= 0.0f || screenHeight <= 0.0f)
    {
        return Ray{};
    }

    float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / screenHeight);

    XMMATRIX invViewProj = XMMatrixInverse(nullptr, view * proj);

    XMVECTOR nearPointScreen = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR farPointScreen  = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

    XMVECTOR nearPointWorld = XMVector3TransformCoord(nearPointScreen, invViewProj);
    XMVECTOR farPointWorld  = XMVector3TransformCoord(farPointScreen, invViewProj);

    XMVECTOR rayDirVec = XMVector3Normalize(farPointWorld - nearPointWorld);

    Ray ray;
    XMStoreFloat3(&ray.origin, nearPointWorld);
    XMStoreFloat3(&ray.direction, rayDirVec);

    return ray;
}

bool EditorRaycast::RayIntersectAABB(
    const Ray& ray,
    const XMFLOAT3& boxMin,
    const XMFLOAT3& boxMax,
    float& outDistance)
{
    float tMin = 0.0f;
    float tMax = 1e30f;

    const float* rayOrigin = &ray.origin.x;
    const float* rayDir = &ray.direction.x;
    const float* bMin = &boxMin.x;
    const float* bMax = &boxMax.x;

    for (int i = 0; i < 3; ++i)
    {
        if (std::abs(rayDir[i]) < 1e-8f)
        {
            if (rayOrigin[i] < bMin[i] || rayOrigin[i] > bMax[i])
            {
                return false;
            }
        }
        else
        {
            float invD = 1.0f / rayDir[i];
            float t1 = (bMin[i] - rayOrigin[i]) * invD;
            float t2 = (bMax[i] - rayOrigin[i]) * invD;

            if (t1 > t2) std::swap(t1, t2);

            tMin = (std::max)(tMin, t1);
            tMax = (std::min)(tMax, t2);

            if (tMin > tMax) return false;
        }
    }

    outDistance = tMin;
    return true;
}

CObject* EditorRaycast::PickObject(
    float screenX, float screenY,
    float screenWidth, float screenHeight,
    Camera* camera,
    const std::vector<std::vector<std::unique_ptr<CObject>>>& objectList,
    int& outTagIndex,
    int& outObjectIndex)
{
    outTagIndex = -1;
    outObjectIndex = -1;

    if (!camera) return nullptr;

    XMMATRIX view = camera->GetView();
    XMMATRIX proj = camera->GetProj();

    Ray ray = CreateRayFromScreen(screenX, screenY, screenWidth, screenHeight, view, proj);

    float closestDist = 1e30f;
    CObject* bestHitObj = nullptr;

    for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
    {
        const auto& objVec = objectList[tagIdx];
        for (size_t objIdx = 0; objIdx < objVec.size(); ++objIdx)
        {
            const auto& obj = objVec[objIdx];
            if (!obj || obj->GetIsDestroyed()) continue;

            CObjectInfo* info = obj->GetComponent<CObjectInfo>();
            if (info)
            {
                ObjectTag tag = info->GetObjectTag();
                // Skip camera, fade, manager, or background UI elements if desired
                if (tag == ObjectTag::CAMERA || tag == ObjectTag::FADE || tag == ObjectTag::MANAGER)
                {
                    continue;
                }
            }

            CTransform* transform = obj->GetComponent<CTransform>();
            if (!transform) continue;

            XMFLOAT3 pos = transform->GetPos();
            XMFLOAT3 scale = transform->GetScale();
            XMFLOAT3 boxMin, boxMax;

            BoxCollider3D* collider = obj->GetComponent<BoxCollider3D>();
            if (collider)
            {
                XMFLOAT3 colSize = collider->GetSize();
                XMFLOAT3 colOffset = collider->GetOffset();

                XMFLOAT3 center = {
                    pos.x + colOffset.x,
                    pos.y + colOffset.y,
                    pos.z + colOffset.z
                };
                XMFLOAT3 halfExtents = {
                    (std::max)(0.2f, colSize.x * scale.x * 0.5f),
                    (std::max)(0.2f, colSize.y * scale.y * 0.5f),
                    (std::max)(0.2f, colSize.z * scale.z * 0.5f)
                };

                boxMin = { center.x - halfExtents.x, center.y - halfExtents.y, center.z - halfExtents.z };
                boxMax = { center.x + halfExtents.x, center.y + halfExtents.y, center.z + halfExtents.z };
            }
            else
            {
                // Default bounding box fallback for objects without colliders
                XMFLOAT3 halfExtents = {
                    (std::max)(0.4f, scale.x * 0.5f),
                    (std::max)(0.4f, scale.y * 0.5f),
                    (std::max)(0.4f, scale.z * 0.5f)
                };
                boxMin = { pos.x - halfExtents.x, pos.y - halfExtents.y, pos.z - halfExtents.z };
                boxMax = { pos.x + halfExtents.x, pos.y + halfExtents.y, pos.z + halfExtents.z };
            }

            float dist = 0.0f;
            if (RayIntersectAABB(ray, boxMin, boxMax, dist))
            {
                if (dist < closestDist)
                {
                    closestDist = dist;
                    bestHitObj = obj.get();
                    outTagIndex = static_cast<int>(tagIdx);
                    outObjectIndex = static_cast<int>(objIdx);
                }
            }
        }
    }

    return bestHitObj;
}
