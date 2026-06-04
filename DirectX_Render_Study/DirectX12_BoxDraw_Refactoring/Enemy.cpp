#include "Enemy.h"
//===== インクルード=====


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

#include "BoxCollider3D.h"


Enemy::Enemy(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::ENEMY);


	//----- Model -----
	//モチE  のパスの初期匁E
	ModelPath = "Assets/Model/OffensiveIdle.glb";

	//ステージで使うモデルを一覧にしてどこかでロードしておくといいかも
	//モチE  のローチE
	CModel* model = GetComponent<CModel>();


	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({ 0.0f, 1.0f, 0.0f });
	collider->SetSize({ 1.0f, 2.0f, 1.0f });

}

void Enemy::Update()
{



}
