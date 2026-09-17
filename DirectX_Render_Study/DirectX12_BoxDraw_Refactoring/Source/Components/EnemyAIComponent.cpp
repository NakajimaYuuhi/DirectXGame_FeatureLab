#include "EnemyAIComponent.h"
#include "Object.h"
#include "Transform.h"
#include "Model.h"
#include "Player.h"
#include "ObjectManager.h"
#include "CharacterMovementComponent.h"
#include "GravityComponent.h"
#include "HealthComponent.h"
#include "EnemyCounter.h"
#include <cmath>

EnemyAIComponent::EnemyAIComponent()
	: CComponent("EnemyAIComponent")
{
}

void EnemyAIComponent::Start()
{
	if (m_Owner)
	{
		m_movementComp = m_Owner->GetComponent<CharacterMovementComponent>();
		m_gravityComp = m_Owner->GetComponent<GravityComponent>();
		m_healthComp = m_Owner->GetComponent<HealthComponent>();
	}

	ChangeState(EnemyAIState::Idle);
}

void EnemyAIComponent::Update(float deltaTime)
{
	UpdateState(deltaTime);
}

void EnemyAIComponent::ChangeState(EnemyAIState newState)
{
	m_currentState = newState;

	if (!m_Owner) return;
	CModel* model = m_Owner->GetComponent<CModel>();

	switch (m_currentState)
	{
	case EnemyAIState::Idle:
		if (model) model->PlayAnimation("Idle");
		break;
	case EnemyAIState::Chase:
		if (model) model->PlayAnimation("Run");
		break;
	case EnemyAIState::Attack:
		m_attackTimer = 0.0f;
		m_hasAttacked = false;
		if (model) model->PlayAnimation("Attack", false);
		break;
	case EnemyAIState::Hurt:
		m_hurtTimer = 0.0f;
		if (model) model->PlayAnimation("RecieveHit", false);
		break;
	case EnemyAIState::Dead:
		m_deadTimer = 0.0f;
		if (model) model->PlayAnimation("Death", false);
		break;
	}
}

void EnemyAIComponent::UpdateState(float deltaTime)
{
	switch (m_currentState)
	{
	case EnemyAIState::Idle:
		UpdateIdle(deltaTime);
		break;
	case EnemyAIState::Chase:
		UpdateChase(deltaTime);
		break;
	case EnemyAIState::Attack:
		UpdateAttack(deltaTime);
		break;
	case EnemyAIState::Hurt:
		UpdateHurt(deltaTime);
		break;
	case EnemyAIState::Dead:
		UpdateDead(deltaTime);
		break;
	}
}

void EnemyAIComponent::UpdateIdle(float deltaTime)
{
	Player* player = ObjectManager::GetInstance().GetPlayer();
	if (player && !player->IsDead() && m_Owner)
	{
		CTransform* transform = m_Owner->GetComponent<CTransform>();
		if (!transform) return;

		DirectX::XMFLOAT3 myPos = transform->GetPos();
		DirectX::XMFLOAT3 playerPos = player->GetPos();

		float dx = playerPos.x - myPos.x;
		float dz = playerPos.z - myPos.z;
		float distSq = dx * dx + dz * dz;

		if (distSq <= DETECTION_RANGE * DETECTION_RANGE)
		{
			ChangeState(EnemyAIState::Chase);
		}
	}
}

void EnemyAIComponent::UpdateChase(float deltaTime)
{
	Player* player = ObjectManager::GetInstance().GetPlayer();
	if (player && !player->IsDead() && m_Owner)
	{
		CTransform* transform = m_Owner->GetComponent<CTransform>();
		if (!transform) return;

		DirectX::XMFLOAT3 myPos = transform->GetPos();
		DirectX::XMFLOAT3 playerPos = player->GetPos();

		float dx = playerPos.x - myPos.x;
		float dz = playerPos.z - myPos.z;
		float distSq = dx * dx + dz * dz;

		if (distSq <= ATTACK_RANGE * ATTACK_RANGE)
		{
			ChangeState(EnemyAIState::Attack);
			return;
		}

		if (m_movementComp)
		{
			m_movementComp->MoveTowards(playerPos, deltaTime);
		}
	}
	else
	{
		ChangeState(EnemyAIState::Idle);
	}
}

void EnemyAIComponent::UpdateAttack(float deltaTime)
{
	m_attackTimer += deltaTime;

	Player* player = ObjectManager::GetInstance().GetPlayer();
	if (player && m_Owner)
	{
		CTransform* transform = m_Owner->GetComponent<CTransform>();
		if (transform)
		{
			DirectX::XMFLOAT3 myPos = transform->GetPos();
			DirectX::XMFLOAT3 playerPos = player->GetPos();
			float dx = playerPos.x - myPos.x;
			float dz = playerPos.z - myPos.z;

			if (dx != 0.0f || dz != 0.0f)
			{
				float targetRotY = atan2f(dx, dz);
				transform->SetRotation({ 0.0f, targetRotY, 0.0f });
			}

			if (!m_hasAttacked && m_attackTimer >= HIT_TIMING)
			{
				float distSq = dx * dx + dz * dz;
				if (distSq <= (ATTACK_RANGE + 0.3f) * (ATTACK_RANGE + 0.3f) && !player->IsDead())
				{
					player->TakeDamage(1);
				}
				m_hasAttacked = true;
			}
		}
	}

	if (m_attackTimer >= ATTACK_DURATION)
	{
		if (player && !player->IsDead())
		{
			ChangeState(EnemyAIState::Chase);
		}
		else
		{
			ChangeState(EnemyAIState::Idle);
		}
	}
}

void EnemyAIComponent::UpdateHurt(float deltaTime)
{
	m_hurtTimer += deltaTime;
	if (m_hurtTimer >= HURT_DURATION)
	{
		ChangeState(EnemyAIState::Chase);
	}
}

void EnemyAIComponent::UpdateDead(float deltaTime)
{
	m_deadTimer += deltaTime;
	if (m_deadTimer >= DEATH_DURATION)
	{
		OnDie();
	}
}

void EnemyAIComponent::OnDamaged()
{
	ChangeState(EnemyAIState::Hurt);
}

void EnemyAIComponent::OnDie()
{
	if (!m_Owner) return;

	m_Owner->SetIsDestroyed(true);

	EnemyCounter* enemyCounter = (EnemyCounter*)ObjectManager::GetInstance().GetManager("EnemyCounter");
	if (enemyCounter)
	{
		enemyCounter->Defeat();
	}

	C3D_Object* billBoard = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::EFFECT, "Explosion"));
	if (billBoard)
	{
		CTransform* transform = billBoard->GetComponent<CTransform>();
		CTransform* myTransform = m_Owner->GetComponent<CTransform>();
		if (transform && myTransform)
		{
			transform->SetPos(myTransform->GetPos());
			transform->SetScale({ 0.8f, 1.0f, 0.8f });
		}
	}
}
