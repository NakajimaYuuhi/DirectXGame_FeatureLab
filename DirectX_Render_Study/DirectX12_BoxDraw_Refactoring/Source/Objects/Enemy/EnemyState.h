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
public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};

// Enemy Dead State
class EnemyDeadState : public StateBase<Enemy>
{
public:
	void OnEnter() override;
	void OnUpdate(float deltaTime) override;
	void OnExit() override;
};
