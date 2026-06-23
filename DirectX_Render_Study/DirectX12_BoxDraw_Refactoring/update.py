import os

player_cpp = '''#include "Player.h"
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
		Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));

		DirectX::XMFLOAT3 bulletPos = GetPos();
		bullet->SetTransform({bulletPos.x,bulletPos.y + 1.4f, bulletPos.z}, {0.1f, 0.1f, 0.1f}, {0.0f, 0.0f, 0.0f});

		CModel* Bullet_Model = bullet->GetComponent<CModel>();

		auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
		Bullet_Model->CopyFrom(sharedModel);

		bullet->SetDirection(GetFront());
	}

	CModel* model = GetComponent<CModel>();
	if (model) {
		model->UpdateAnimation(0.016f);
	}
}
'''

camera_h = '''#pragma once
#include "3D_Object.h"
#include "StringAlias.h"

class Player;

class Camera : public C3D_Object
{
public:
	Camera(String _Name);
	~Camera() = default;

	void Init() override;
	void Update() override;

	DirectX::XMMATRIX GetView() { return view; }
	DirectX::XMMATRIX GetProj() { return proj; }
	float GetAngleY() const { return m_angleY; }

private:
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

	Player* m_player = nullptr;
	float m_angleY = 0.0f;
	float m_distance = 5.0f;
	float m_height = 2.5f;
	float m_rotationSpeed = 0.02f;
};
'''

camera_cpp = '''#include "Camera.h"
#include "Player.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "BasicSettings.h"
#include <cmath>

Camera::Camera(String _Name)
	:C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::CAMERA);

	view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0, 2.5, -5, 1),
		DirectX::XMVectorSet(0, 2, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));

	proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		0.1f,
		1000.0f);
}

void Camera::Init()
{
	m_player = ObjectManager::GetInstance().GetPlayer();
}

void Camera::Update()
{
	if (CInputManager::GetInstance().IsKeyPress('Q'))
	{
		m_angleY -= m_rotationSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('E'))
	{
		m_angleY += m_rotationSpeed;
	}

	if (m_player)
	{
		DirectX::XMFLOAT3 playerPos = m_player->GetPos();

		float offsetX = sinf(m_angleY) * m_distance;
		float offsetZ = -cosf(m_angleY) * m_distance;

		DirectX::XMVECTOR camPos = DirectX::XMVectorSet(playerPos.x + offsetX, playerPos.y + m_height, playerPos.z + offsetZ, 1.0f);
		DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(playerPos.x, playerPos.y + 1.0f, playerPos.z, 1.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

		view = DirectX::XMMatrixLookAtLH(camPos, targetPos, up);
	}
}
'''

with open('Player.cpp', 'w', encoding='utf-8') as f:
    f.write(player_cpp)

with open('Camera.h', 'w', encoding='utf-8') as f:
    f.write(camera_h)

with open('Camera.cpp', 'w', encoding='utf-8') as f:
    f.write(camera_cpp)
