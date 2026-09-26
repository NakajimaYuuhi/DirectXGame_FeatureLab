#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Camera;
class CharacterMovementComponent;
class GravityComponent;
class HealthComponent;

enum class PlayerControllerState
{
	Idle,
	Move,
	Attack,
	Hurt,
	Dead
};

class PlayerControllerComponent : public CComponent
{
public:
	PlayerControllerComponent();
	virtual ~PlayerControllerComponent() = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Input; }

	// Input checks
	bool HasMoveInput() const;

	// Actions
	void PerformAttack();

	// State notifications (called by HealthComponent callbacks etc.)
	void OnDamaged();
	void OnDie();

	// State getter
	PlayerControllerState GetCurrentState() const { return m_currentState; }
	void ChangeState(PlayerControllerState newState);

private:
	void UpdateState(float deltaTime);
	void UpdateIdle(float deltaTime);
	void UpdateMove(float deltaTime);
	void UpdateAttack(float deltaTime);
	void UpdateHurt(float deltaTime);
	void UpdateDead(float deltaTime);

	DirectX::XMFLOAT3 CalculateMoveDirection() const;

private:
	PlayerControllerState m_currentState = PlayerControllerState::Idle;

	// Attack timers
	float m_attackTimer = 0.0f;
	bool  m_hasAttacked = false;
	const float ATTACK_DURATION = 0.9f;
	const float CAST_TIMING = 0.35f;

	// Hurt timer
	float m_hurtTimer = 0.0f;
	const float HURT_DURATION = 0.6f;

	// Dead timer
	float m_deadTimer = 0.0f;
	bool  m_hasTransitioned = false;
	const float DEATH_DURATION = 1.2f;

	// Cached components and camera
	Camera*                     m_camera = nullptr;
	CharacterMovementComponent* m_movementComp = nullptr;
	GravityComponent*           m_gravityComp = nullptr;
	HealthComponent*            m_healthComp = nullptr;
};
