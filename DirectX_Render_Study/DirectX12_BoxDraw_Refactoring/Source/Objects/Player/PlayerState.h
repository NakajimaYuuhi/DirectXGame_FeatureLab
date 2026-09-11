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

// Player Attack State (Spell1)
class PlayerAttackState : public StateBase<Player>
{
private:
	float m_attackTimer = 0.0f;
	bool m_hasAttacked = false;
	const float ATTACK_DURATION = 0.9f;
	const float CAST_TIMING = 0.35f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Player Hurt State (RecieveHit)
class PlayerHurtState : public StateBase<Player>
{
private:
	float m_hurtTimer = 0.0f;
	const float HURT_DURATION = 0.6f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Player Dead State (Death)
class PlayerDeadState : public StateBase<Player>
{
private:
	float m_deadTimer = 0.0f;
	bool m_hasTransitioned = false;
	const float DEATH_DURATION = 1.2f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

