#include "PlayerState.h"
#include "Player.h"
#include "InputManager.h"
#include "Model.h"

// --- PlayerIdleState ---
void PlayerIdleState::OnEnter()
{
	if (!owner) return;
	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Idle");
	}
}

void PlayerIdleState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	// 攻撃入力チェック
	if (CInputManager::GetInstance().IsKeyTrigger('I'))
	{
		owner->GetStateMachine().ChangeState(std::make_shared<PlayerAttackState>());
		return;
	}

	// 移動入力チェック
	if (owner->HasMoveInput())
	{
		owner->GetStateMachine().ChangeState(std::make_shared<PlayerMoveState>());
		return;
	}
}

void PlayerIdleState::OnExit()
{
}

// --- PlayerMoveState ---
void PlayerMoveState::OnEnter()
{
	if (!owner) return;
	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Run");
	}
}

void PlayerMoveState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	// 攻撃入力チェック
	if (CInputManager::GetInstance().IsKeyTrigger('I'))
	{
		owner->GetStateMachine().ChangeState(std::make_shared<PlayerAttackState>());
		return;
	}

	// 移動入力がない場合は IdleState へ
	if (!owner->HasMoveInput())
	{
		owner->GetStateMachine().ChangeState(std::make_shared<PlayerIdleState>());
		return;
	}

	owner->ProcessMovement(deltaTime);
}

void PlayerMoveState::OnExit()
{
}

// --- PlayerAttackState ---
void PlayerAttackState::OnEnter()
{
	if (!owner) return;
	m_attackTimer = 0.0f;
	owner->PerformAttack();
}

void PlayerAttackState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_attackTimer += deltaTime;
	if (m_attackTimer >= ATTACK_DURATION)
	{
		if (owner->HasMoveInput())
		{
			owner->GetStateMachine().ChangeState(std::make_shared<PlayerMoveState>());
		}
		else
		{
			owner->GetStateMachine().ChangeState(std::make_shared<PlayerIdleState>());
		}
	}
}

void PlayerAttackState::OnExit()
{
}

// --- PlayerHurtState ---
void PlayerHurtState::OnEnter()
{
	if (!owner) return;
	m_hurtTimer = 0.0f;

	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("RecieveHit", false);
	}
}

void PlayerHurtState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_hurtTimer += deltaTime;
	if (m_hurtTimer >= HURT_DURATION)
	{
		if (owner->HasMoveInput())
		{
			owner->GetStateMachine().ChangeState(std::make_shared<PlayerMoveState>());
		}
		else
		{
			owner->GetStateMachine().ChangeState(std::make_shared<PlayerIdleState>());
		}
	}
}

void PlayerHurtState::OnExit()
{
}

// --- PlayerDeadState ---
void PlayerDeadState::OnEnter()
{
	if (!owner) return;
	m_deadTimer = 0.0f;
	m_hasTransitioned = false;

	CModel* model = owner->GetComponent<CModel>();
	if (model)
	{
		model->PlayAnimation("Death", false);
	}
}

void PlayerDeadState::OnUpdate(float deltaTime)
{
	if (!owner) return;

	m_deadTimer += deltaTime;
	if (!m_hasTransitioned && m_deadTimer >= DEATH_DURATION)
	{
		m_hasTransitioned = true;
		owner->OnDie();
	}
}

void PlayerDeadState::OnExit()
{
}

