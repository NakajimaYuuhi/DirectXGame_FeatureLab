#include "BulletComponent.h"
#include "Object.h"
#include "Transform.h"
#include "ObjectInfo.h"
#include "HealthComponent.h"
#include "ObjectManager.h"

BulletComponent::BulletComponent()
	: CComponent("BulletComponent")
{
}

void BulletComponent::Init()
{
}

void BulletComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	// 寿命管理
	m_elapsedTime += deltaTime;
	if (m_lifeTime > 0.0f && m_elapsedTime >= m_lifeTime)
	{
		m_Owner->SetIsDestroyed(true);
		return;
	}

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (transform)
	{
		DirectX::XMFLOAT3 pos = transform->GetPos();

		// 移動計算 (フレーム基準 60fps)
		float frameFactor = deltaTime > 0.0f ? (deltaTime * 60.0f) : 1.0f;
		pos.x += m_direction.x * m_speed * frameFactor;
		pos.y += m_direction.y * m_speed * frameFactor;
		pos.z += m_direction.z * m_speed * frameFactor;
		transform->SetPos(pos);

		// UVスクロール
		DirectX::XMFLOAT2 currentUV = transform->GetUVOffset();
		currentUV.x += 0.01f * frameFactor;
		transform->SetUVOffset(currentUV);
	}
}

void BulletComponent::OnCollision(CObject* _Other)
{
	if (!_Other || !m_Owner) return;

	CObjectInfo* otherInfo = _Other->GetComponent<CObjectInfo>();
	if (otherInfo && otherInfo->GetObjectTag() == m_targetTag)
	{
		m_Owner->SetIsDestroyed(true);

		HealthComponent* health = _Other->GetComponent<HealthComponent>();
		if (health)
		{
			health->TakeDamage(m_damage);
		}

		// パティクル生成
		CTransform* transform = m_Owner->GetComponent<CTransform>();
		DirectX::XMFLOAT3 pos = transform ? transform->GetPos() : DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };

		for (int i = 0; i < 20; ++i)
		{
			CObject* particle = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BILLBOARD, "RandomParticle");
			if (particle)
			{
				CTransform* pTransform = particle->GetComponent<CTransform>();
				if (pTransform)
				{
					pTransform->SetPos(pos);
					pTransform->SetScale({ 0.05f, 1.0f, 0.05f });
				}
			}
		}
	}
}
