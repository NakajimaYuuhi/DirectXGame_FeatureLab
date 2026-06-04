//===== インクルード =====

#include "Player.h"

//Model
#include "Model.h"
//ObjectInfo
#include "ObjectInfo.h"


Player::Player(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER);


	//----- Model -----
	//モデルのパスの初期化
	ModelPath = "Assets/Model/OffensiveIdle.glb";

	//モデルのロード
	CModel* model = GetComponent<CModel>();

	model->ModelLoad(ModelPath);

}

void Player::Update()
{

}