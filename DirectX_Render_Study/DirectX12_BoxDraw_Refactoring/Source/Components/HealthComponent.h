#pragma once
#include "Component.h"
#include <functional>

class HealthComponent : public CComponent
{
public:
	HealthComponent();
	HealthComponent(int maxHp, float invincibleDuration = 1.5f, float blinkInterval = 0.08f);
	virtual ~HealthComponent() = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::PostPhysics; }

	// Damage handling
	void TakeDamage(int damage);

	// State getters
	int GetHP() const { return m_hp; }
	int GetMaxHP() const { return m_maxHp; }
	bool IsDead() const { return m_hp <= 0; }
	bool IsInvincible() const { return m_invincibleTimer > 0.0f; }

	// Setters
	void SetMaxHP(int maxHp, bool resetCurrentHp = true);
	void SetHP(int hp);
	void SetInvincibleDuration(float duration) { m_invincibleDuration = duration; }
	void SetBlinkInterval(float interval) { m_blinkInterval = interval; }

	// Callbacks
	void SetOnDamagedCallback(std::function<void(int currentHp, int maxHp)> callback) { m_onDamaged = callback; }
	void SetOnDieCallback(std::function<void()> callback) { m_onDie = callback; }

private:
	int   m_hp = 10;
	int   m_maxHp = 10;
	float m_invincibleTimer = 0.0f;
	float m_invincibleDuration = 1.5f;
	float m_blinkInterval = 0.08f;

	std::function<void(int, int)> m_onDamaged;
	std::function<void()>         m_onDie;
};
