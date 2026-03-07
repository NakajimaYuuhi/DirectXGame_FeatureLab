#include "Collision.h"

#include <DirectXMath.h>

#include "Collider_Ray.h"
#include "Collider_Plane.h"

#include "Mesh.h"

//bool CCollision::CheckCollision
//(   
//    const CCollider_Ray& ray,
//    const CCollider_Plane& plane,
//    float& outT
//)
//{
//    using namespace DirectX;
//
//
//    XMVECTOR ray_start  = XMLoadFloat3(&ray.GetStart()); 
//    XMVECTOR ray_dir    = XMLoadFloat3(&ray.GetDirection());
//
//    XMVECTOR plane_normal   = XMLoadFloat3(&plane.GetNormal());
//    XMVECTOR plane_pos      = XMLoadFloat3(&plane.GetPos());
//
//    //平面のNormalとRayの方向ベクトルで内積
//    //値は本来スカラーだが、Vectorで返ってくるので、GetXで取得
//    //0だった場合は並行
//    //denomは分母の意味
//    float denom = XMVectorGetX(XMVector3Dot(plane_normal, ray_dir));
//
//    // 平行チェック
//    if (fabs(denom) < 0.0001f)
//        return false;
//
//    XMVECTOR P0minusO = plane_pos - ray_start;
//
//    float t = XMVectorGetX(XMVector3Dot(plane_normal, P0minusO)) / denom;
//
//    if (t < 0.0f)
//        return false; 
//
//    outT = t;
//    return true;
//}

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


bool PointInTriangle_Cross(
    const DirectX::XMVECTOR& P,
    const DirectX::XMVECTOR& A,
    const DirectX::XMVECTOR& B,
    const DirectX::XMVECTOR& C)
{
    using namespace DirectX;

    XMVECTOR N = XMVector3Normalize(XMVector3Cross(B - A, C - A));

    XMVECTOR C1 = XMVector3Dot(XMVector3Cross(P - A, B - A), N);
    XMVECTOR C2 = XMVector3Dot(XMVector3Cross(P - B, C - B), N);
    XMVECTOR C3 = XMVector3Dot(XMVector3Cross(P - C, A - C), N);

    float c1 = XMVectorGetX(C1);
    float c2 = XMVectorGetX(C2);
    float c3 = XMVectorGetX(C3);

    bool pos = (c1 >= 0 && c2 >= 0 && c3 >= 0);
    bool neg = (c1 <= 0 && c2 <= 0 && c3 <= 0);

    return pos || neg;
}


bool CCollision::CheckPointInQuad(DirectX::XMFLOAT3 _CollisionPoint, CMesh* _Mesh)
{
    using namespace DirectX;


    //Meshの頂点情報の取得
    DirectX::XMVECTOR tmpCollision = XMLoadFloat3(&_CollisionPoint);
    std::vector<DirectX::XMFLOAT3> vertices;
    std::vector<uint16_t> indices;
    
    _Mesh->GetVertex(vertices, indices);

    DirectX::XMMATRIX world = _Mesh->GetWorld();


    for (size_t i = 0; i < indices.size(); i += 3)
    {
        XMVECTOR A = XMVector3Transform(XMLoadFloat3(&vertices[indices[i]]), world);
        XMVECTOR B = XMVector3Transform(XMLoadFloat3(&vertices[indices[i + 1]]), world);
        XMVECTOR C = XMVector3Transform(XMLoadFloat3(&vertices[indices[i + 2]]), world);

        if (PointInTriangle_Cross(tmpCollision, A, B, C))
        {
            return true;    // この三角形の内部！
        }
    }


    return false;
}
