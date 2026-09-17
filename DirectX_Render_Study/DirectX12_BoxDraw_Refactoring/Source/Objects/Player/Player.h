#pragma once
#include "3D_Object.h"
#include "StringAlias.h"
#include "StateMachine.h"
#include <memory>

class Camera;
class GravityComponent;
class HealthComponent;
class CharacterMovementComponent;

class Player : public C3D_Object
{
public:
	Player(String _Name);
	~Player() = default;

	void Init() override;
	void Awake() override;
	void Start() override;
	virtual void Update() override;

	// StateMachine getter
	StateMachine<Player>& GetStateMachine() { return m_stateMachine; }

	// Helper methods for state execution
	bool HasMoveInput() const;
	void ProcessMovement(float deltaTime);
	void SnapToGround();
	void PerformAttack();
	bool IsGrounded() const;

	// HP / Damage management (delegated to HealthComponent)
	int GetHP() const;
	int GetMaxHP() const;
	bool IsDead() const;
	void TakeDamage(int damage);
	bool IsInvincible() const;
	void OnDie();

	// Component getters
	GravityComponent* GetGravityComponent() const { return m_gravityComp; }
	HealthComponent*  GetHealthComponent() const { return m_healthComp; }

protected:
	String ModelPath;
	float Speed = 0.1f;
	Camera* m_camera = nullptr;

	// Attached components
	GravityComponent*           m_gravityComp = nullptr;
	HealthComponent*            m_healthComp = nullptr;
	CharacterMovementComponent* m_movementComp = nullptr;

	StateMachine<Player> m_stateMachine;
};