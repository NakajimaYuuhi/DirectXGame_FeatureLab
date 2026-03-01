#include "Collider_Ray.h"
#include "Mesh.h"

//Rayの更新
void CCollider_Ray::Update()
{
	DirectX::XMFLOAT3 meshPos = m_pMesh->GetPos();
	DirectX::XMFLOAT3 meshScale = m_pMesh->GetScale();

	//Rayの始点だけを一旦更新
	m_Start.x = meshPos.x;
	m_Start.y = meshPos.y - meshScale.y * 0.5f;
	m_Start.z = meshPos.z;


	//方向を正規化する
	DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&m_Direction);
	dir = DirectX::XMVector3Normalize(dir);
	DirectX::XMStoreFloat3(&m_Direction, dir);

}
