#include "HealthComponent.h"
#include "Object.h"
#include <cmath>

HealthComponent::HealthComponent()
	: CComponent("HealthComponent")
{
}

HealthComponent::HealthComponent(int maxHp, float invincibleDuration, float blinkInterval)
	: CComponent("HealthComponent"),
	  m_hp(maxHp),
	  m_maxHp(maxHp),
	  m_invincibleDuration(invincibleDuration),
	  m_blinkInterval(blinkInterval)
{
}

void HealthComponent::Start()
{
	m_hp = m_maxHp;
	m_invincibleTimer = 0.0f;
	if (m_Owner)
	{
		m_Owner->SetVisible(true);
	}
}

void HealthComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	if (m_invincibleTimer > 0.0f)
	{
		m_invincibleTimer -= deltaTime;
		if (m_invincibleTimer <= 0.0f)
		{
			m_invincibleTimer = 0.0f;
			m_Owner->SetVisible(true);
		}
		else
		{
			bool visible = (fmodf(m_invincibleTimer, m_blinkInterval * 2.0f) >= m_blinkInterval);
			m_Owner->SetVisible(visible);
		}
	}
}

void HealthComponent::TakeDamage(int damage)
{
	if (m_hp <= 0 || IsInvincible()) return;

	m_hp -= damage;
	if (m_hp < 0)
	{
		m_hp = 0;
	}

	m_invincibleTimer = m_invincibleDuration;

	if (m_hp <= 0)
	{
		if (m_onDie)
		{
			m_onDie();
		}
	}
	else
	{
		if (m_onDamaged)
		{
			m_onDamaged(m_hp, m_maxHp);
		}
	}
}

void HealthComponent::SetMaxHP(int maxHp, bool resetCurrentHp)
{
	m_maxHp = maxHp;
	if (resetCurrentHp)
	{
		m_hp = maxHp;
	}
}

void HealthComponent::SetHP(int hp)
{
	m_hp = hp;
	if (m_hp > m_maxHp)
	{
		m_hp = m_maxHp;
	}
}
