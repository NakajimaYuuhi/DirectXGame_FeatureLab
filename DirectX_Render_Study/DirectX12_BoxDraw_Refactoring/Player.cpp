//===== インクルーチE=====

#include "Player.h"

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


Player::Player(String _Name)
	:C3D_Object(_Name)
{
	//----- タグ -----
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER);


	//----- Model -----
	//モチE  のパスの初期匁E
	ModelPath = "Assets/Model/OffensiveIdle.glb";

	//モチE  のローチE
	CModel* model = GetComponent<CModel>();


	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({0.0f, 1.0f, 0.0f});
	collider->SetSize({ 1.0f, 2.0f, 1.0f });

}

void Player::Update()
{
	//奥
	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		DirectX::XMFLOAT3 pos = GetPos();

		//本来は、DeltaTimeを掛けるべきだが、今回は仮なので固定値で移動させる
		DirectX::XMFLOAT3 Forward = GetFront();
		DirectX::XMFLOAT3 Movement = { Forward.x * Speed, Forward.y * Speed, Forward.z * Speed };

		DirectX::XMFLOAT3 newPos = { pos.x + Movement.x, pos.y + Movement.y, pos.z + Movement.z };
		SetPos(newPos);
	}

	//手前
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		DirectX::XMFLOAT3 pos = GetPos();

		//本来は、DeltaTimeを掛けるべきだが、今回は仮なので固定値で移動させる
		DirectX::XMFLOAT3 Forward = GetFront();
		DirectX::XMFLOAT3 Movement = { Forward.x * -Speed, Forward.y * -Speed, Forward.z * -Speed };

		DirectX::XMFLOAT3 newPos = { pos.x + Movement.x, pos.y + Movement.y, pos.z + Movement.z };
		SetPos(newPos);
	}

	//右
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		DirectX::XMFLOAT3 pos = GetPos();

		//本来は、DeltaTimeを掛けるべきだが、今回は仮なので固定値で移動させる
		DirectX::XMFLOAT3 Right = GetRight();
		DirectX::XMFLOAT3 Movement = { Right.x * Speed, Right.y * Speed, Right.z * Speed };

		DirectX::XMFLOAT3 newPos = { pos.x + Movement.x, pos.y + Movement.y, pos.z + Movement.z };
		SetPos(newPos);
	}

	//左
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		DirectX::XMFLOAT3 pos = GetPos();

		//本来は、DeltaTimeを掛けるべきだが、今回は仮なので固定値で移動させる
		DirectX::XMFLOAT3 Right = GetRight();
		DirectX::XMFLOAT3 Movement = { Right.x * -Speed, Right.y * -Speed, Right.z * -Speed };

		DirectX::XMFLOAT3 newPos = { pos.x + Movement.x, pos.y + Movement.y, pos.z + Movement.z };
		SetPos(newPos);


	}


	//ぶっ飛 E
	if (CInputManager::GetInstance().IsKeyTrigger('F'))
	{
		Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scene::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));

		DirectX::XMFLOAT3 bulletPos = GetPos();
		bullet->SetTransform({bulletPos.x,bulletPos.y + 1.4f, bulletPos.z}, {0.1f, 0.1f, 0.1f}, {0.0f, 0.0f, 0.0f});

		CModel* Bullet_Model = bullet->GetComponent<CModel>();

		auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
		Bullet_Model->CopyFrom(sharedModel);

		//弾の方向をプレイヤーの前方に設定
		bullet->SetDirection(GetFront());
	}


}
