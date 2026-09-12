#pragma once
#include "3D_Object.h"
#include "StringAlias.h"
#include "StateMachine.h"
#include <memory>

class Camera;

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
	void PerformAttack();

	// HP / Damage management
	int GetHP() const { return HP; }
	int GetMaxHP() const { return MaxHP; }
	bool IsDead() const { return HP <= 0; }
	void TakeDamage(int damage);
	bool IsInvincible() const { return m_invincibleTimer > 0.0f; }
	void OnDie();

protected:
	String ModelPath;
	float Speed = 0.1f;
	int HP = 10;
	int MaxHP = 10;
	float m_invincibleTimer = 0.0f;
	const float INVINCIBLE_DURATION = 1.5f;
	const float BLINK_INTERVAL = 0.08f;
	Camera* m_camera = nullptr;

	StateMachine<Player> m_stateMachine;
};