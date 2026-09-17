#pragma once
#include "3D_Object.h"
#include "StateMachine.h"
#include <memory>

class GravityComponent;
class HealthComponent;
class CharacterMovementComponent;

class Enemy : public C3D_Object
{
public:
	Enemy(String _Name);
	~Enemy() = default;

	virtual void Init() override;
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update() override;
	virtual void OnCollision(CObject* _Other) override;

	// StateMachine getter
	StateMachine<Enemy>& GetStateMachine() { return m_stateMachine; }

	// Helper methods for state execution
	void MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime);
	void SnapToGround();
	bool IsGrounded() const;
	void OnDie();

	// HP / Damage management (delegated to HealthComponent)
	int GetHP() const;
	void TakeDamage(int damage);

	// Component getters
	GravityComponent*           GetGravityComponent() const { return m_gravityComp; }
	HealthComponent*            GetHealthComponent() const { return m_healthComp; }
	CharacterMovementComponent* GetMovementComponent() const { return m_movementComp; }

protected:
	String ModelPath;
	float Speed = 0.05f;

	GravityComponent*           m_gravityComp = nullptr;
	HealthComponent*            m_healthComp = nullptr;
	CharacterMovementComponent* m_movementComp = nullptr;

	StateMachine<Enemy> m_stateMachine;
};