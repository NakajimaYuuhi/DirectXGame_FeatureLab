//===== インクルード =====

#include "Player.h"

//Model
#include "Model.h"
//ObjectInfo
#include "ObjectInfo.h"

#include "Transform.h"
#include "Bullet.h"


#include "InputManager.h"

//ObjectManager
#include "ObjectManager.h"


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


	//ぶっ飛び
	if (CInputManager::GetInstance().IsKeyTrigger('F'))
	{
		Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scene::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));

		bullet->SetTransform({GetPos()}, {0.1f, 0.1f, 0.1f}, {0.0f, 0.0f, 0.0f});

		CModel* Bullet_Model = bullet->GetComponent<CModel>();

		Bullet_Model->ModelLoad("Assets/Model/cube.glb");

		//弾の方向をプレイヤーの前方に設定
		bullet->SetDirection(GetFront());
	}


}