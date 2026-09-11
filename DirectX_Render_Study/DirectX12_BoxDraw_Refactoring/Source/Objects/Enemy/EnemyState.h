#pragma once
#include "StateBase.h"

class Enemy;

// Enemy Idle State
class EnemyIdleState : public StateBase<Enemy>
{
private:
	const float DETECTION_RANGE = 15.0f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Enemy Chase State
class EnemyChaseState : public StateBase<Enemy>
{
private:
	const float ATTACK_RANGE = 2.0f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Enemy Attack State (Attack)
class EnemyAttackState : public StateBase<Enemy>
{
private:
	float m_attackTimer = 0.0f;
	bool m_hasAttacked = false;
	const float ATTACK_DURATION = 0.8f;
	const float HIT_TIMING = 0.3f;
	const float ATTACK_RANGE = 2.2f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Enemy Hurt State (RecieveHit)
class EnemyHurtState : public StateBase<Enemy>
{
private:
	float m_hurtTimer = 0.0f;
	const float HURT_DURATION = 0.4f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Enemy Dead State (Death)
class EnemyDeadState : public StateBase<Enemy>
{
private:
	float m_deadTimer = 0.0f;
	const float DEATH_DURATION = 1.0f;

public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};