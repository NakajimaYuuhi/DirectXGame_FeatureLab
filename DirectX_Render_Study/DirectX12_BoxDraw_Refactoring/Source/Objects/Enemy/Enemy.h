#pragma once
#include "3D_Object.h"
#include "StateMachine.h"
#include <memory>

class Enemy : public C3D_Object
{
public:
	Enemy(String _Name);
	~Enemy() = default;

	virtual void Init() override;
	virtual void Update() override;
	virtual void OnCollision(CObject* _Other) override;

	// StateMachine getter
	StateMachine<Enemy>& GetStateMachine() { return m_stateMachine; }

	// Helper methods for state execution
	void MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime);
	void OnDie();
	int GetHP() const { return HP; }
	void TakeDamage(int damage);

protected:
	String ModelPath;
	float Speed = 0.05f;
	int HP = 3;
	float m_flashTimer = 0.0f;
	const float FLASH_DURATION = 0.3f;
	const float BLINK_INTERVAL = 0.06f;

	StateMachine<Enemy> m_stateMachine;
};
