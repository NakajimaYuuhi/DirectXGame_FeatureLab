#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Player;
class CharacterMovementComponent;
class GravityComponent;
class HealthComponent;

enum class EnemyAIState
{
	Idle,
	Chase,
	Attack,
	Hurt,
	Dead
};

class EnemyAIComponent : public CComponent
{
public:
	EnemyAIComponent();
	virtual ~EnemyAIComponent() = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::AI; }

	// State notifications
	void OnDamaged();
	void OnDie();

	// State getter
	EnemyAIState GetCurrentState() const { return m_currentState; }
	void ChangeState(EnemyAIState newState);

private:
	void UpdateState(float deltaTime);
	void UpdateIdle(float deltaTime);
	void UpdateChase(float deltaTime);
	void UpdateAttack(float deltaTime);
	void UpdateHurt(float deltaTime);
	void UpdateDead(float deltaTime);

private:
	EnemyAIState m_currentState = EnemyAIState::Idle;

	const float DETECTION_RANGE = 15.0f;
	const float ATTACK_RANGE    = 2.0f;

	// Attack timer
	float m_attackTimer = 0.0f;
	bool  m_hasAttacked = false;
	const float ATTACK_DURATION = 0.8f;
	const float HIT_TIMING = 0.3f;

	// Hurt timer
	float m_hurtTimer = 0.0f;
	const float HURT_DURATION = 0.4f;

	// Dead timer
	float m_deadTimer = 0.0f;
	const float DEATH_DURATION = 1.0f;

	// Cached components
	CharacterMovementComponent* m_movementComp = nullptr;
	GravityComponent*           m_gravityComp = nullptr;
	HealthComponent*            m_healthComp = nullptr;
};
