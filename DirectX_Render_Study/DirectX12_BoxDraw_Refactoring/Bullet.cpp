#include "Bullet.h"

#include "ObjectInfo.h"
#include "Transform.h"

#include "Enemy.h"

#include "BoxCollider3D.h"

#include "ObjectManager.h"


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

void Bullet::OnCollision(CObject* _Other)
{
	//衝突した相手がEnemyだったら消える
	CObjectInfo* otherInfo = _Other->GetComponent<CObjectInfo>();
	if (otherInfo && otherInfo->GetObjectTag() == ObjectTag::ENEMY)
	{
		SetIsDestroyed(true);

		//パーティクルを出す(20個)
		for (int i = 0; i < 20; i++)
		{
			CObject* particle = ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::BILLBOARD, "RandomParticle");
			if (particle)
			{
				CTransform* transform = particle->GetComponent<CTransform>();
				//位置
				DirectX::XMFLOAT3 pos = GetPos();
				transform->SetPos(pos);
				transform->SetScale({ 0.05f, 1.0f, 0.05f });
			}
		}
	}
}
