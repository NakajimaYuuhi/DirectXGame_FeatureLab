#include "PlayerControllerComponent.h"
#include "Object.h"
#include "Transform.h"
#include "Model.h"
#include "ModelManager.h"
#include "Camera.h"
#include "BulletComponent.h"
#include "audio.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "CharacterMovementComponent.h"
#include "GravityComponent.h"
#include "HealthComponent.h"
#include "Source/Core/Scenes/Manager/SceneManager.h"
#include <cmath>

PlayerControllerComponent::PlayerControllerComponent()
	: CComponent("PlayerControllerComponent")
{
}

void PlayerControllerComponent::Start()
{
	m_camera = ObjectManager::GetInstance().GetCamera();

	if (m_Owner)
	{
		m_movementComp = m_Owner->GetComponent<CharacterMovementComponent>();
		m_gravityComp = m_Owner->GetComponent<GravityComponent>();
		m_healthComp = m_Owner->GetComponent<HealthComponent>();
	}

	ChangeState(PlayerControllerState::Idle);
}

void PlayerControllerComponent::Update(float deltaTime)
{
	// Jump input
	if (m_gravityComp && m_gravityComp->IsGrounded() && CInputManager::GetInstance().IsKeyTrigger(VK_SPACE))
	{
		m_gravityComp->Jump();
	}

	UpdateState(deltaTime);
}

bool PlayerControllerComponent::HasMoveInput() const
{
	int moveX = 0;
	int moveZ = 0;
	if (CInputManager::GetInstance().IsKeyPress('D')) moveX += 1;
	if (CInputManager::GetInstance().IsKeyPress('A')) moveX -= 1;
	if (CInputManager::GetInstance().IsKeyPress('W')) moveZ += 1;
	if (CInputManager::GetInstance().IsKeyPress('S')) moveZ -= 1;

	return (moveX != 0 || moveZ != 0);
}

DirectX::XMFLOAT3 PlayerControllerComponent::CalculateMoveDirection() const
{
	Camera* cam = m_camera ? m_camera : ObjectManager::GetInstance().GetCamera();
	if (!cam) return { 0.0f, 0.0f, 0.0f };

	float angleY = cam->GetAngleY();
	float s = sinf(angleY);
	float c = cosf(angleY);

	DirectX::XMFLOAT3 forward = { -s, 0.0f, c };
	DirectX::XMFLOAT3 right   = {  c, 0.0f, s };

	DirectX::XMFLOAT3 dir = { 0.0f, 0.0f, 0.0f };

	if (CInputManager::GetInstance().IsKeyPress('W'))
	{
		dir.x += forward.x;
		dir.z += forward.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('S'))
	{
		dir.x -= forward.x;
		dir.z -= forward.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('D'))
	{
		dir.x += right.x;
		dir.z += right.z;
	}
	if (CInputManager::GetInstance().IsKeyPress('A'))
	{
		dir.x -= right.x;
		dir.z -= right.z;
	}

	float len = sqrtf(dir.x * dir.x + dir.z * dir.z);
	if (len > 0.001f)
	{
		dir.x /= len;
		dir.z /= len;
	}

	return dir;
}

void PlayerControllerComponent::ChangeState(PlayerControllerState newState)
{
	m_currentState = newState;

	if (!m_Owner) return;
	CModel* model = m_Owner->GetComponent<CModel>();

	switch (m_currentState)
	{
	case PlayerControllerState::Idle:
		if (model) model->PlayAnimation("Idle");
		break;
	case PlayerControllerState::Move:
		if (model) model->PlayAnimation("Run");
		break;
	case PlayerControllerState::Attack:
		m_attackTimer = 0.0f;
		m_hasAttacked = false;
		if (model) model->PlayAnimation("Spell1", false);
		break;
	case PlayerControllerState::Hurt:
		m_hurtTimer = 0.0f;
		if (model) model->PlayAnimation("RecieveHit", false);
		break;
	case PlayerControllerState::Dead:
		m_deadTimer = 0.0f;
		m_hasTransitioned = false;
		if (model) model->PlayAnimation("Death", false);
		break;
	}
}

void PlayerControllerComponent::UpdateState(float deltaTime)
{
	switch (m_currentState)
	{
	case PlayerControllerState::Idle:
		UpdateIdle(deltaTime);
		break;
	case PlayerControllerState::Move:
		UpdateMove(deltaTime);
		break;
	case PlayerControllerState::Attack:
		UpdateAttack(deltaTime);
		break;
	case PlayerControllerState::Hurt:
		UpdateHurt(deltaTime);
		break;
	case PlayerControllerState::Dead:
		UpdateDead(deltaTime);
		break;
	}
}

void PlayerControllerComponent::UpdateIdle(float deltaTime)
{
	// Attack check
	if (CInputManager::GetInstance().IsKeyTrigger('I') || CInputManager::GetInstance().IsKeyTrigger('J'))
	{
		ChangeState(PlayerControllerState::Attack);
		return;
	}

	// Move check
	if (HasMoveInput())
	{
		ChangeState(PlayerControllerState::Move);
		return;
	}
}

void PlayerControllerComponent::UpdateMove(float deltaTime)
{
	// Attack check
	if (CInputManager::GetInstance().IsKeyTrigger('I') || CInputManager::GetInstance().IsKeyTrigger('J'))
	{
		ChangeState(PlayerControllerState::Attack);
		return;
	}

	// Move check
	if (!HasMoveInput())
	{
		ChangeState(PlayerControllerState::Idle);
		return;
	}

	// Process movement
	DirectX::XMFLOAT3 dir = CalculateMoveDirection();
	if (m_movementComp)
	{
		m_movementComp->MoveDirection(dir, deltaTime);
	}
}

void PlayerControllerComponent::UpdateAttack(float deltaTime)
{
	m_attackTimer += deltaTime;

	if (!m_hasAttacked && m_attackTimer >= CAST_TIMING)
	{
		PerformAttack();
		m_hasAttacked = true;
	}

	if (m_attackTimer >= ATTACK_DURATION)
	{
		if (HasMoveInput())
		{
			ChangeState(PlayerControllerState::Move);
		}
		else
		{
			ChangeState(PlayerControllerState::Idle);
		}
	}
}

void PlayerControllerComponent::UpdateHurt(float deltaTime)
{
	m_hurtTimer += deltaTime;
	if (m_hurtTimer >= HURT_DURATION)
	{
		if (HasMoveInput())
		{
			ChangeState(PlayerControllerState::Move);
		}
		else
		{
			ChangeState(PlayerControllerState::Idle);
		}
	}
}

void PlayerControllerComponent::UpdateDead(float deltaTime)
{
	m_deadTimer += deltaTime;
	if (!m_hasTransitioned && m_deadTimer >= DEATH_DURATION)
	{
		m_hasTransitioned = true;
		OnDie();
	}
}

void PlayerControllerComponent::PerformAttack()
{
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	Audio* audio = m_Owner->GetComponent<Audio>();

	CObject* bulletObj = ObjectManager::GetInstance().Instantiate(Scenes::ID::GAME, ObjectTag::PLAYER_BULLET, "PlayerBullet");
	if (bulletObj && transform)
	{
		DirectX::XMFLOAT3 pos = transform->GetPos();
		DirectX::XMFLOAT3 front = transform->GetFront();

		CTransform* bulletTransform = bulletObj->GetComponent<CTransform>();
		if (bulletTransform)
		{
			bulletTransform->SetPos({ pos.x + front.x * 0.6f, pos.y + 0.8f, pos.z + front.z * 0.6f });
			bulletTransform->SetScale({ 0.15f, 0.15f, 0.15f });
			bulletTransform->SetRotation({ 0.0f, 0.0f, 0.0f });
		}

		BulletComponent* bulletComp = bulletObj->GetComponent<BulletComponent>();
		if (bulletComp)
		{
			bulletComp->SetDirection(front);
		}
	}

	if (audio)
	{
		audio->Play();
	}
}

void PlayerControllerComponent::OnDamaged()
{
	ChangeState(PlayerControllerState::Hurt);
}

void PlayerControllerComponent::OnDie()
{
	SceneManager::GetInstance().ChangeSceneWithFade(Scenes::ID::Failed, 0.5f);
}
