#include "EnemyState.h"
#include "Enemy.h"
#include "Player.h"
#include "ObjectManager.h"
#include "Model.h"
#include <cmath>

// --- EnemyIdleState ---
void EnemyIdleState::OnEnter()
{
}

void EnemyIdleState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	Player* playerObj = ObjectManager::GetInstance().GetPlayer();

	if (playerObj)
	{
		DirectX::XMFLOAT3 myPos = owner->GetPos();
		DirectX::XMFLOAT3 playerPos = playerObj->GetPos();

		float dx = playerPos.x - myPos.x;
		float dz = playerPos.z - myPos.z;
		float distSq = dx * dx + dz * dz;

		if (distSq <= DETECTION_RANGE * DETECTION_RANGE)
		{
			owner->GetStateMachine().ChangeState(std::make_shared<EnemyChaseState>());
			return;
		}
	}
}

void EnemyIdleState::OnExit()
{
}

// --- EnemyChaseState ---
void EnemyChaseState::OnEnter()
{
	if (!owner) return;
	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Run");
	}
}

void EnemyChaseState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	Player* playerObj = ObjectManager::GetInstance().GetPlayer();

	if (playerObj)
	{
		DirectX::XMFLOAT3 myPos = owner->GetPos();
		DirectX::XMFLOAT3 playerPos = playerObj->GetPos();

		float dx = playerPos.x - myPos.x;
		float dz = playerPos.z - myPos.z;
		float distSq = dx * dx + dz * dz;

		if (distSq <= ATTACK_RANGE * ATTACK_RANGE)
		{
			owner->GetStateMachine().ChangeState(std::make_shared<EnemyAttackState>());
			return;
		}

		owner->MoveTowards(playerPos, deltaTime);
	}
	else
	{
		owner->GetStateMachine().ChangeState(std::make_shared<EnemyIdleState>());
	}
}

void EnemyChaseState::OnExit()
{
}

// --- EnemyAttackState ---
void EnemyAttackState::OnEnter()
{
	if (!owner) return;
	m_attackTimer = 0.0f;
	m_hasAttacked = false;

	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Attack");
	}
}

void EnemyAttackState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_attackTimer += deltaTime;

	Player* playerObj = ObjectManager::GetInstance().GetPlayer();
	if (playerObj)
	{
		DirectX::XMFLOAT3 myPos = owner->GetPos();
		DirectX::XMFLOAT3 playerPos = playerObj->GetPos();
		float dx = playerPos.x - myPos.x;
		float dz = playerPos.z - myPos.z;
		if (dx != 0.0f || dz != 0.0f)
		{
			float targetRotY = atan2f(dx, dz);
			owner->SetRotation({ 0.0f, targetRotY, 0.0f });
		}

		if (!m_hasAttacked && m_attackTimer >= HIT_TIMING)
		{
			float distSq = dx * dx + dz * dz;
			if (distSq <= ATTACK_RANGE * ATTACK_RANGE)
			{
				playerObj->TakeDamage(1);
			}
			m_hasAttacked = true;
		}
	}

	if (m_attackTimer >= ATTACK_DURATION)
	{
		owner->GetStateMachine().ChangeState(std::make_shared<EnemyChaseState>());
	}
}

void EnemyAttackState::OnExit()
{
}

// --- EnemyHurtState ---
void EnemyHurtState::OnEnter()
{
	if (!owner) return;
	m_hurtTimer = 0.0f;

	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("RecieveHit");
	}
}

void EnemyHurtState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_hurtTimer += deltaTime;
	if (m_hurtTimer >= HURT_DURATION)
	{
		owner->GetStateMachine().ChangeState(std::make_shared<EnemyChaseState>());
	}
}

void EnemyHurtState::OnExit()
{
}

// --- EnemyDeadState ---
void EnemyDeadState::OnEnter()
{
	if (!owner) return;
	m_deadTimer = 0.0f;

	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Death");
	}
}

void EnemyDeadState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_deadTimer += deltaTime;
	if (m_deadTimer >= DEATH_DURATION)
	{
		owner->OnDie();
	}
}

void EnemyDeadState::OnExit()
{
}