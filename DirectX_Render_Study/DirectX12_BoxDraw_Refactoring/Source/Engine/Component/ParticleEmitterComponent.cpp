#include "ParticleEmitterComponent.h"
#include "Object.h"
#include "Transform.h"
#include "ObjectInfo.h"
#include "ObjectManager.h"

ParticleEmitterComponent::ParticleEmitterComponent()
	: CComponent("ParticleEmitterComponent")
{
}

void ParticleEmitterComponent::Start()
{
	if (m_burstOnStart && !m_hasBurstOnStart)
	{
		Emit();
		m_hasBurstOnStart = true;
	}
}

void ParticleEmitterComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	if (m_spawnInterval > 0.0f)
	{
		m_timer += deltaTime;
		if (m_timer >= m_spawnInterval)
		{
			Emit();
			m_timer = 0.0f;
		}
	}
}

void ParticleEmitterComponent::Emit()
{
	if (!m_Owner || m_particlePrefab.empty()) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	DirectX::XMFLOAT3 pos = transform ? transform->GetPos() : DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < m_burstCount; ++i)
	{
		CObject* particle = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BILLBOARD, m_particlePrefab);
		if (particle)
		{
			CTransform* pTransform = particle->GetComponent<CTransform>();
			if (pTransform)
			{
				pTransform->SetPos(pos);
				pTransform->SetScale(m_particleScale);
			}
			particle->Awake();
		}
	}
}
