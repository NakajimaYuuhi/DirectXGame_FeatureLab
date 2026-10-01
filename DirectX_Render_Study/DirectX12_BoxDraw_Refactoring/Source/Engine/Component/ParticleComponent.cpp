#include "ParticleComponent.h"
#include "Object.h"
#include "Transform.h"
#include <cstdlib>

ParticleComponent::ParticleComponent()
	: CComponent("ParticleComponent")
{
}

void ParticleComponent::InitRandomDirection(float speed, float lifeTime)
{
	m_speed = speed;
	m_lifeTime = lifeTime;
	m_maxLifeTime = lifeTime;
	m_isRandomDirection = true;

	m_direction.x = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
	m_direction.y = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
	m_direction.z = static_cast<float>(rand()) / RAND_MAX * 2.0f - 1.0f;
}

void ParticleComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	if (m_lifeTime <= 0.0f)
	{
		m_Owner->SetIsDestroyed(true);
		return;
	}

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (transform)
	{
		DirectX::XMFLOAT3 pos = transform->GetPos();
		float step = m_speed * (deltaTime * 60.0f);
		pos.x += m_direction.x * step;
		pos.y += m_direction.y * step;
		pos.z += m_direction.z * step;
		transform->SetPos(pos);
	}

	m_lifeTime -= deltaTime;
}
