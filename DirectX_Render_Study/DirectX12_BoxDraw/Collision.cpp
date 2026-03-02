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


    XMVECTOR ray_start  = XMLoadFloat3(&ray.GetStart()); 
    XMVECTOR ray_dir    = XMLoadFloat3(&ray.GetDirection());

    XMVECTOR plane_normal   = XMLoadFloat3(&plane.GetNormal());
    XMVECTOR plane_pos      = XMLoadFloat3(&plane.GetPos());

    //平面のNormalとRayの方向ベクトルで内積
    //値は本来スカラーだが、Vectorで返ってくるので、GetXで取得
    //0だった場合は並行
    //denomは分母の意味
    float denom = XMVectorGetX(XMVector3Dot(plane_normal, ray_dir));

    // 平行チェック
    if (fabs(denom) < 0.0001f)
        return false;

    XMVECTOR P0minusO = plane_pos - ray_start;

    float t = XMVectorGetX(XMVector3Dot(plane_normal, P0minusO)) / denom;

    if (t < 0.0f)
        return false; 

    outT = t;
    return true;
}

bool CCollision::CheckCollision(const CCollider_Ray& ray, const CCollider_Plane& plane, float& outT, DirectX::XMFLOAT3& _CollisionPoint)
{
    using namespace DirectX;


    XMVECTOR ray_start = XMLoadFloat3(&ray.GetStart());
    XMVECTOR ray_dir = XMLoadFloat3(&ray.GetDirection());

    XMVECTOR plane_normal = XMLoadFloat3(&plane.GetNormal());
    XMVECTOR plane_pos = XMLoadFloat3(&plane.GetPos());

    //平面のNormalとRayの方向ベクトルで内積
    //値は本来スカラーだが、Vectorで返ってくるので、GetXで取得
    //0だった場合は並行
    //denomは分母の意味
    float denom = XMVectorGetX(XMVector3Dot(plane_normal, ray_dir));

    // 平行チェック
    if (fabs(denom) < 0.0001f)
        return false;

    XMVECTOR P0minusO = plane_pos - ray_start;

    float t = XMVectorGetX(XMVector3Dot(plane_normal, P0minusO)) / denom;

    if (t < 0.0f)
        return false;

    outT = t;
	//衝突点の
	XMVECTOR collision_point = XMVectorAdd(ray_start, XMVectorScale(ray_dir, t));
	XMStoreFloat3(&_CollisionPoint, collision_point);

    return true;
}
