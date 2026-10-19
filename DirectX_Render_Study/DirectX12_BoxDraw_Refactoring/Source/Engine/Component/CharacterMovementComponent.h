#pragma once
#include "EngineAPI.h"
#include "Component.h"
#include <DirectXMath.h>

class FieldComponent;
class GravityComponent;

class CharacterMovementComponent : public CComponent
{
public:
	ENGINE_API CharacterMovementComponent();
	ENGINE_API CharacterMovementComponent(float speed);
	virtual ~CharacterMovementComponent() = default;

	ENGINE_API void Start() override;
	ENGINE_API void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Movement; }

	// Movement methods
	ENGINE_API void Move(const DirectX::XMFLOAT3& movement, float deltaTime);
	ENGINE_API void MoveDirection(const DirectX::XMFLOAT3& direction, float deltaTime);
	ENGINE_API void MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime);

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

	FieldComponent*   m_field = nullptr;
	GravityComponent* m_gravityComp = nullptr;
};
