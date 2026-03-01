#include "Collision.h"

#include "Collider_Ray.h"
#include "Collider_Plane.h"

void CCollision::CheckCollision
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

    float d = plane.GetDistance();

    float denom = XMVectorGetX(XMVector3Dot(N, D));

    if (fabs(denom) < 0.0001f)
        return false; // •½s

    float numer = XMVectorGetX(XMVector3Dot(N, O)) + d;

    float t = -numer / denom;

    if (t < 0)
        return false; // ‹t•ûŒü

    outT = t;
    return true;
}
