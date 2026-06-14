#include "Enemy.h"
//===== インクルード=====


#include "ObjectInfo.h"

//Model
#include "Model.h"
#include "ModelManager.h"
//ObjectInfo
#include "ObjectInfo.h"

#include "Transform.h"
#include "Bullet.h"


#include "InputManager.h"

//ObjectManager
#include "ObjectManager.h"

#include "Transform.h"
#include "BoxCollider3D.h"

#include "Explosion.h"


Enemy::Enemy(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::ENEMY);


	//----- Model -----
	//モチE  のパスの初期匁E
	//ModelPath = "Assets/Model/OffensiveIdle.glb";
	ModelPath = "Assets/Model/uploads_files_4381633_GreenExplosion.glb"; 
	//ステージで使うモデルを一覧にしてどこかでロードしておくといいかも
	//モチE  のローチE
	CModel* model = GetComponent<CModel>();


	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);
    model->PlayAnimation(0);

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({ 0.0f, 1.0f, 0.0f });
	collider->SetSize({ 1.0f, 2.0f, 1.0f });

	SetScale({-1.0f,-1.0f,-1.0f});
}

void Enemy::Update()
{
    CModel* model = GetComponent<CModel>();
    if (model) {
       //model->UpdateAnimation(0.016f); // ~60 FPS
		model->UpdateAnimation(0.016f);
    }
}

void Enemy::OnCollision(CObject* _Other)
{
	//衝突した相手がBulletだったら消える
	CObjectInfo* otherInfo = _Other->GetComponent<CObjectInfo>();

	//タグがPlayerBulletだったら
	if (otherInfo && otherInfo->GetObjectTag() == ObjectTag::PLAYER_BULLET)
	{
		//HPを減らす
		HP--;
		if (HP <= 0)
		{
			SetIsDestroyed(true);
			
			//explosionを生成
			C3D_Object* billBoard = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::EFFECT, "Explosion"));
			if (billBoard)
			{
				CTransform* transform = billBoard->GetComponent<CTransform>();
				CTransform* bulletTransform = _Other->GetComponent<CTransform>();
				//位置
				DirectX::XMFLOAT3 pos = bulletTransform->GetPos();
				transform->SetPos(pos);
				transform->SetScale({ 0.8f, 1.0f, 0.8f });
			}

		}
	}
}
