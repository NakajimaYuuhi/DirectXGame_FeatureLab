#include "Skydome.h"
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

#include "Transform.h"
#include "BoxCollider3D.h"

#include "Explosion.h"


Skydome::Skydome(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::BACKGROUND);


	//----- Model -----
	//ModelPath = "Assets/Model/OffensiveIdle.glb";
	//ModelPath = "Assets/Model/free_-_skybox_anime_sky.glb";
	ModelPath = "Assets/Model/uploads_files_4381633_GreenExplosion.glb";

	//ステージで使うモデルを一覧にしてどこかでロードしておくといいかも

	CModel* model = GetComponent<CModel>();


	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	

	model->CopyFrom(sharedModel);
	model->PlayAnimation(0);

	SetScale({ 1.0f,1.0f,1.0f });
}

void Skydome::Update()
{
}


