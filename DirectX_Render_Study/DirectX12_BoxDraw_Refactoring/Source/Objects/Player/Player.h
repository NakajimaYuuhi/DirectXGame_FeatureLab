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
	virtual void Update() override;

	// StateMachine getter
	StateMachine<Player>& GetStateMachine() { return m_stateMachine; }

	// Helper methods for state execution
	bool HasMoveInput() const;
	void ProcessMovement(float deltaTime);
	void PerformAttack();

protected:
	String ModelPath;
	float Speed = 0.1f;
	Camera* m_camera = nullptr;

	StateMachine<Player> m_stateMachine;
};
