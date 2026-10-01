#pragma once
#include "Component.h"
#include <directxmath.h>

class ParticleComponent : public CComponent
{
public:
	ParticleComponent();
	~ParticleComponent() override = default;

	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Movement; }

	void SetDirection(const DirectX::XMFLOAT3& dir) { m_direction = dir; }
	DirectX::XMFLOAT3 GetDirection() const { return m_direction; }

	void SetSpeed(float speed) { m_speed = speed; }
	float GetSpeed() const { return m_speed; }

	void SetLifeTime(float lifeTime) { m_lifeTime = lifeTime; m_maxLifeTime = lifeTime; }
	float GetLifeTime() const { return m_lifeTime; }

	void InitRandomDirection(float speed = 0.05f, float lifeTime = 1.0f);
	bool IsRandomDirection() const { return m_isRandomDirection; }

private:
	DirectX::XMFLOAT3 m_direction = { 0.0f, 0.0f, 0.0f };
	float m_speed = 0.05f;
	float m_lifeTime = 1.0f;
	float m_maxLifeTime = 1.0f;
	bool m_isRandomDirection = false;
};
