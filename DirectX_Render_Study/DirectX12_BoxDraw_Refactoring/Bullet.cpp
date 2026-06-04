#include "Bullet.h"

#include "ObjectInfo.h"
#include "Transform.h"

#include "BoxCollider3D.h"


Bullet::Bullet(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER_BULLET);

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetSize({ 0.1f, 0.1f, 0.1f });

}

void Bullet::Update()
{
	//Transformの取得
	CTransform* transform = GetComponent<CTransform>();

	//移動
	DirectX::XMFLOAT3 pos = transform->GetPos();
	
	//移動量の計算
	DirectX::XMFLOAT3 moveAmount = {
		Direction.x * Speed,
		Direction.y * Speed,
		Direction.z * Speed
	};

	//新しい位置の計算
	pos.x += moveAmount.x;
	pos.y += moveAmount.y;
	pos.z += moveAmount.z;

	//位置のセット
	transform->SetPos(pos);



}