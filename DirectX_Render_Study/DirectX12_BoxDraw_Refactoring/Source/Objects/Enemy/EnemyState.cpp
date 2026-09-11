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

	// Player‚ÌŒŸõ
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
		DirectX::XMFLOAT3 playerPos = playerObj->GetPos();
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

// --- EnemyDeadState ---
void EnemyDeadState::OnEnter()
{
	if (!owner) return;
	owner->OnDie();
}

void EnemyDeadState::OnUpdate(float deltaTime)
{
}

void EnemyDeadState::OnExit()
{
}
