#pragma once
#include "Collider3D.h"
#include <DirectXMath.h>
#include "Object.h"
#include "Transform.h"

class BoxCollider3D : public Collider3D
{
public:
	//コンストラクタ
	BoxCollider3D()
	{
		ColliderType = Collider::BOX_3D;
	}

	//デストラクタ
	virtual ~BoxCollider3D(){}



	//WorldPosの取得
	virtual DirectX::XMFLOAT3 GetWorldPos()
	{
		//Colliderの位置は、オブジェクトの位置 + オフセット
		DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		if (m_Owner)
		{
			CTransform* transform = m_Owner->GetComponent<CTransform>();
			if (transform)
			{
				pos = transform->GetPos();
			}
		}
		pos.x += Offset.x;
		pos.y += Offset.y;
		pos.z += Offset.z;
		return pos;
	}

	//Getter
	DirectX::XMFLOAT3  GetSize() const { return Size; }
	//Setter
	void SetSize(DirectX::XMFLOAT3 _Size) { Size = _Size; }


protected:
	DirectX::XMFLOAT3 	Size = { 1.0f, 1.0f, 1.0f };	//大きさ
};

