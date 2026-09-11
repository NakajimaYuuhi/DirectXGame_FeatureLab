#include "Player.h"
#include "PlayerState.h"
#include "Camera.h"
#include "Model.h"
#include "ModelManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Bullet.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "BoxCollider3D.h"
#include "TimeManager.h"
#include "audio.h"
#include <cmath>

Player::Player(String _Name)
	: C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::PLAYER);

	ModelPath = "Assets/Model/Wizard.glb";

	CModel* model = GetComponent<CModel>();
	auto sharedModel = ModelManager::GetInstance().GetModel(ModelPath);

	model->CopyFrom(sharedModel);
	model->PlayAnimation("Run");

	BoxCollider3D* collider = AddComponent<BoxCollider3D>();
	collider->SetOffset({ 0.0f, 0.75f, 0.0f });
	collider->SetSize({ 0.6f, 1.5f, 0.6f });

	Audio* audio = AddComponent<Audio>();
	audio->Load("Assets/Audio/SE/Fire1.wav");
}

void Player::Init()
{
	m_camera = ObjectManager::GetInstance().GetCamera();
	SetScale({ 0.5f, 0.5f, 0.5f });

	m_stateMachine.SetOwner(this);
	m_stateMachine.ChangeState(std::make_shared<PlayerIdleState>());
}

void Player::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();

	CModel* model = GetComponent<CModel>();
	if (model)
	{
		model->UpdateAnimation(dt);
	}

	m_stateMachine.OnUpdate(dt);
}

bool Player::HasMoveInput() const
{
	return (CInputManager::GetInstance().IsKeyPress('W') ||
		CInputManager::GetInstance().IsKeyPress('S') ||
		CInputManager::GetInstance().IsKeyPress('A') ||
		CInputManager::GetInstance().IsKeyPress('D'));
}

void Player::ProcessMovement(float deltaTime)
{
	if (!m_camera) return;

	float angleY = m_camera->GetAngleY();
	float s = sinf(angleY);
	float c = cosf(angleY);

	DirectX::XMFLOAT3 forward = { -s, 0.0f, c };
	DirectX::XMFLOAT3 right = { c, 0.0f, s };

	DirectX::XMFLOAT3 pos = GetPos();
	DirectX::XMFLOAT3 movement = { 0.0f, 0.0f, 0.0f };

	float frameSpeed = Speed * (deltaTime * 60.0f);

	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		movement.x += forward.x * frameSpeed;
		movement.z += forward.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		movement.x -= forward.x * frameSpeed;
		movement.z -= forward.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		movement.x += right.x * frameSpeed;
		movement.z += right.z * frameSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		movement.x -= right.x * frameSpeed;
		movement.z -= right.z * frameSpeed;
	}

	if (movement.x != 0.0f || movement.z != 0.0f)
	{
		DirectX::XMFLOAT3 newPos = { pos.x + movement.x, pos.y + movement.y, pos.z + movement.z };
		SetPos(newPos);

		float targetRotY = atan2f(movement.x, movement.z);
		SetRotation({ 0.0f, targetRotY, 0.0f });
	}
}

void Player::PerformAttack()
{
	Audio* audio = GetComponent<Audio>();

	Bullet* bullet = (Bullet*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::GAME, ObjectTag::PLAYER_BULLET, "Bullet"));
	if (bullet)
	{
		DirectX::XMFLOAT3 bulletPos = GetPos();
		bullet->SetTransform({ bulletPos.x, bulletPos.y + 1.0f, bulletPos.z }, { 0.1f, 0.1f, 0.1f }, { 0.0f, 0.0f, 0.0f });

		CModel* Bullet_Model = bullet->GetComponent<CModel>();
		if (Bullet_Model)
		{
			auto sharedModel = ModelManager::GetInstance().GetModel("Assets/Model/cube.glb");
			Bullet_Model->CopyFrom(sharedModel);
		}

		bullet->SetDirection(GetFront());
	}

	if (audio)
	{
		audio->Play();
	}
}

void Player::TakeDamage(int damage)
{
	HP -= damage;
	if (HP < 0)
	{
		HP = 0;
	}

	OutputDebugStringA(("Player Took Damage! Current HP: " + std::to_string(HP) + "\n").c_str());
}