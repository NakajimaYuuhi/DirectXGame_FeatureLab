#include "EditorRaycast.h"
#include "Object.h"
#include "Transform.h"
#include "BoxCollider3D.h"
#include "ObjectInfo.h"
#include "CameraComponent.h"
#include <algorithm>

using namespace DirectX;

Ray EditorRaycast::CreateRayFromScreen(
    float screenX, float screenY,
    float screenWidth, float screenHeight,
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& proj)
{
    float ndcX = (2.0f * screenX / screenWidth) - 1.0f;
    float ndcY = 1.0f - (2.0f * screenY / screenHeight);

    XMMATRIX viewProj = XMMatrixMultiply(view, proj);
    XMVECTOR det;
    XMMATRIX invViewProj = XMMatrixInverse(&det, viewProj);

    XMVECTOR nearPointNdc = XMVectorSet(ndcX, ndcY, 0.0f, 1.0f);
    XMVECTOR farPointNdc  = XMVectorSet(ndcX, ndcY, 1.0f, 1.0f);

    XMVECTOR nearWorld = XMVector3TransformCoord(nearPointNdc, invViewProj);
    XMVECTOR farWorld  = XMVector3TransformCoord(farPointNdc, invViewProj);

    XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(farWorld, nearWorld));

    Ray ray;
    XMStoreFloat3(&ray.origin, nearWorld);
    XMStoreFloat3(&ray.direction, dir);

    return ray;
}

bool EditorRaycast::RayIntersectAABB(
    const Ray& ray,
    const DirectX::XMFLOAT3& boxMin,
    const DirectX::XMFLOAT3& boxMax,
    float& outDistance)
{
    float tMin = 0.0f;
    float tMax = 1e30f;

    const float rayOrigin[3] = { ray.origin.x, ray.origin.y, ray.origin.z };
    const float rayDir[3]    = { ray.direction.x, ray.direction.y, ray.direction.z };
    const float bMin[3]      = { boxMin.x, boxMin.y, boxMin.z };
    const float bMax[3]      = { boxMax.x, boxMax.y, boxMax.z };

    for (int i = 0; i < 3; ++i)
    {
        if (fabsf(rayDir[i]) < 1e-6f)
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
    const DirectX::XMMATRIX& view,
    const DirectX::XMMATRIX& proj,
    const std::vector<std::vector<std::unique_ptr<CObject>>>& objectList,
    int& outTagIndex,
    int& outObjectIndex)
{
    outTagIndex = -1;
    outObjectIndex = -1;

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

CObject* EditorRaycast::PickObject(
    float screenX, float screenY,
    float screenWidth, float screenHeight,
    CameraComponent* camera,
    const std::vector<std::vector<std::unique_ptr<CObject>>>& objectList,
    int& outTagIndex,
    int& outObjectIndex)
{
    if (!camera) return nullptr;
    return PickObject(
        screenX, screenY, screenWidth, screenHeight,
        camera->GetViewMatrix(), camera->GetProjectionMatrix(),
        objectList, outTagIndex, outObjectIndex
    );
}
