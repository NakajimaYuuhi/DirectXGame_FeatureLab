#include "Player.h"
#include "Camera.h"
#include "Model.h"
#include "ModelManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Bullet.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "BoxCollider3D.h"
#include <cmath>

#include "audio.h"

Player::Player(String _Name)
	:C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER);

	ModelPath = "Assets/Model/OffensiveIdle.glb";

	CModel* model = GetComponent<CModel>();
	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);
	model->PlayAnimation(0);

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({0.0f, 1.0f, 0.0f});
	collider->SetSize({ 1.0f, 2.0f, 1.0f });

	Audio* audio = AddComponent<Audio>();
	audio->Load("Assets/Audio/SE/Fire1.wav");

}

void Player::Init()
{
	m_camera = ObjectManager::GetInstance().GetCamera();
}

void Player::Update()
{
	if (!m_camera) return;

	float angleY = m_camera->GetAngleY();
	float s = sinf(angleY);
	float c = cosf(angleY);

	DirectX::XMFLOAT3 forward = { -s, 0.0f, c };
	DirectX::XMFLOAT3 right = { c, 0.0f, s };

	DirectX::XMFLOAT3 pos = GetPos();
	DirectX::XMFLOAT3 movement = { 0.0f, 0.0f, 0.0f };

	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		movement.x += forward.x * Speed;
		movement.z += forward.z * Speed;
	}
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		movement.x -= forward.x * Speed;
		movement.z -= forward.z * Speed;
	}
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		movement.x += right.x * Speed;
		movement.z += right.z * Speed;
	}
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		movement.x -= right.x * Speed;
		movement.z -= right.z * Speed;
	}

	if (movement.x != 0.0f || movement.z != 0.0f)
	{
		DirectX::XMFLOAT3 newPos = { pos.x + movement.x, pos.y + movement.y, pos.z + movement.z };
		SetPos(newPos);
        
        // Face the moving direction
        float targetRotY = atan2f(movement.x, movement.z);
		SetRotation({0.0f, targetRotY, 0.0f});
	}

	if (CInputManager::GetInstance().IsKeyTrigger('F'))
	{
		Audio* audio = GetComponent<Audio>();

		Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));

		DirectX::XMFLOAT3 bulletPos = GetPos();
		bullet->SetTransform({bulletPos.x,bulletPos.y + 1.4f, bulletPos.z}, {0.1f, 0.1f, 0.1f}, {0.0f, 0.0f, 0.0f});

		CModel* Bullet_Model = bullet->GetComponent<CModel>();

		auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
		Bullet_Model->CopyFrom(sharedModel);

		bullet->SetDirection(GetFront());

		audio->Play();
	}

	CModel* model = GetComponent<CModel>();
	if (model) {
		model->UpdateAnimation(0.016f);
	}
}
