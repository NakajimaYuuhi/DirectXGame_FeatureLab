#pragma once
#include "Component.h"
#include "ObjectTag.h"
#include <DirectXMath.h>

class BulletComponent : public CComponent
{
public:
	BulletComponent();
	virtual ~BulletComponent() = default;

	void Init() override;
	void Update(float deltaTime) override;
	void OnCollision(CObject* _Other) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Movement; }

	// Getters & Setters
	void SetDirection(const DirectX::XMFLOAT3& dir) { m_direction = dir; }
	DirectX::XMFLOAT3 GetDirection() const { return m_direction; }

	void SetSpeed(float speed) { m_speed = speed; }
	float GetSpeed() const { return m_speed; }

	void SetLifeTime(float lifeTime) { m_lifeTime = lifeTime; }
	float GetLifeTime() const { return m_lifeTime; }

	void SetDamage(int damage) { m_damage = damage; }
	int GetDamage() const { return m_damage; }

	void SetTargetTag(ObjectTag tag) { m_targetTag = tag; }
	ObjectTag GetTargetTag() const { return m_targetTag; }

private:
	DirectX::XMFLOAT3 m_direction = { 1.0f, 0.0f, 0.0f };
	float m_speed = 0.04f;
	float m_lifeTime = 5.0f;
	float m_elapsedTime = 0.0f;
	int m_damage = 1;
	ObjectTag m_targetTag = ObjectTag::ENEMY;
};
