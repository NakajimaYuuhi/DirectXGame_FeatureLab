#include "Collision.h"

#include <DirectXMath.h>

#include "Collider_Ray.h"
#include "Collider_Plane.h"

bool CCollision::CheckCollision
(   
    const CCollider_Ray& ray,
    const CCollider_Plane& plane,
    float& outT
)
{
    using namespace DirectX;


    XMVECTOR O = XMLoadFloat3(&ray.GetStart());
    XMVECTOR D = XMLoadFloat3(&ray.GetDirection());

    XMVECTOR N = XMLoadFloat3(&plane.GetNormal());
    XMVECTOR P0 = XMLoadFloat3(&plane.GetPos());

    float denom = XMVectorGetX(XMVector3Dot(N, D));

    // 平行チェック
    if (fabs(denom) < 0.0001f)
        return false;

    XMVECTOR P0minusO = P0 - O;

    float t = XMVectorGetX(XMVector3Dot(N, P0minusO)) / denom;

    if (t < 0.0f)
        return false; // 逆方向

    outT = t;
    return true;
}
