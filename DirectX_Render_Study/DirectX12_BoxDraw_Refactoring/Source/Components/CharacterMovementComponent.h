#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Field;
class GravityComponent;

class CharacterMovementComponent : public CComponent
{
public:
	CharacterMovementComponent();
	CharacterMovementComponent(float speed);
	virtual ~CharacterMovementComponent() = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Movement; }

	// Movement methods
	void Move(const DirectX::XMFLOAT3& movement, float deltaTime);
	void MoveDirection(const DirectX::XMFLOAT3& direction, float deltaTime);
	void MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime);

	// Getters and setters
	float GetSpeed() const { return m_speed; }
	void SetSpeed(float speed) { m_speed = speed; }

	float GetMinClimbNormalY() const { return m_minClimbNormalY; }
	void SetMinClimbNormalY(float ny) { m_minClimbNormalY = ny; }

	bool GetAutoRotate() const { return m_autoRotate; }
	void SetAutoRotate(bool autoRotate) { m_autoRotate = autoRotate; }

	const DirectX::XMFLOAT3& GetLastMovement() const { return m_lastMovement; }

private:
	float m_speed = 0.1f;
	float m_minClimbNormalY = 0.45f;
	bool  m_autoRotate = true;

	DirectX::XMFLOAT3 m_lastMovement = { 0.0f, 0.0f, 0.0f };

	Field*            m_field = nullptr;
	GravityComponent* m_gravityComp = nullptr;
};
