#pragma once
#include "StateBase.h"

class Player;

// Player Idle State
class PlayerIdleState : public StateBase<Player>
{
public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Player Move State
class PlayerMoveState : public StateBase<Player>
{
public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Player Attack State
class PlayerAttackState : public StateBase<Player>
{
private:
	float m_attackTimer = 0.0f;
	const float ATTACK_DURATION = 0.2f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};
