#include "Collider_Ray.h"
#include "Mesh.h"

//Rayの更新
void CCollider_Ray::Update()
{
	DirectX::XMFLOAT3 meshPos = m_pMesh->GetPos();
	DirectX::XMFLOAT3 meshScale = m_pMesh->GetScale();

	//Upを取得
	//Upの逆方向にRayの向きを設定
	DirectX::XMFLOAT3 meshUp = m_pMesh->GetUp();


	//----- 方向を直す -----
	//Upの逆方向
	//正規化も行う

	//XMVectorの作成
	DirectX::XMVECTOR VecRayDir = 
		DirectX::XMVector3Normalize(
			DirectX::XMVectorScale(
				DirectX::XMLoadFloat3(&meshUp), -1.0f
			)
		);

	//Directionへの代入
	DirectX::XMStoreFloat3(&m_Direction, VecRayDir);

	//----- 始点を直す -----
	

	//Rayの始点だけを一旦更新
	//TODO:の逆Up方向にScaleYの分を足す
	m_Start.x = meshPos.x + m_Direction.x * (meshScale.y * 0.40f);
	m_Start.y = meshPos.y + m_Direction.y * (meshScale.y * 0.40f);
	m_Start.z = meshPos.z + m_Direction.z * (meshScale.y * 0.40f);


}
