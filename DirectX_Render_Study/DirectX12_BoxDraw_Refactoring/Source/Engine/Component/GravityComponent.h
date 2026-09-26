#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Field;

class GravityComponent : public CComponent
{
public:
	GravityComponent();
	GravityComponent(float gravity, float jumpPower);
	virtual ~GravityComponent() = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Physics; }

	// Jump actions
	void Jump();
	void Jump(float power);

	// Snap directly to terrain ground
	void SnapToGround();

	// Getters and setters
	bool IsGrounded() const { return m_isGrounded; }
	float GetVerticalVelocity() const { return m_verticalVelocity; }
	void SetVerticalVelocity(float vy) { m_verticalVelocity = vy; }

	float GetGravity() const { return m_gravity; }
	void SetGravity(float g) { m_gravity = g; }

	float GetJumpPower() const { return m_jumpPower; }
	void SetJumpPower(float power) { m_jumpPower = power; }

	float GetStepDownLimit() const { return m_stepDownLimit; }
	void SetStepDownLimit(float limit) { m_stepDownLimit = limit; }

private:
	Field* m_field = nullptr;

	float m_verticalVelocity = 0.0f;
	bool  m_isGrounded = false;

	float m_gravity = -25.0f;
	float m_jumpPower = 8.5f;
	float m_terminalVelocity = -30.0f;
	float m_stepDownLimit = 0.4f;
};
