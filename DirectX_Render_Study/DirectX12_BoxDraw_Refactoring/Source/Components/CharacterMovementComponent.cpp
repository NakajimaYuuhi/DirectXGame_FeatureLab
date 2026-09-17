#include "CharacterMovementComponent.h"
#include "Object.h"
#include "Transform.h"
#include "Field.h"
#include "GravityComponent.h"
#include "ObjectManager.h"
#include <cmath>

CharacterMovementComponent::CharacterMovementComponent()
	: CComponent("CharacterMovementComponent")
{
}

CharacterMovementComponent::CharacterMovementComponent(float speed)
	: CComponent("CharacterMovementComponent"), m_speed(speed)
{
}

void CharacterMovementComponent::Start()
{
	m_field = ObjectManager::GetInstance().GetField();
	if (m_Owner)
	{
		m_gravityComp = m_Owner->GetComponent<GravityComponent>();
	}
}

void CharacterMovementComponent::Update(float deltaTime)
{
}

void CharacterMovementComponent::Move(const DirectX::XMFLOAT3& movement, float deltaTime)
{
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	if (movement.x == 0.0f && movement.z == 0.0f)
	{
		return;
	}

	m_lastMovement = movement;

	if (!m_field)
	{
		m_field = ObjectManager::GetInstance().GetField();
	}

	if (!m_gravityComp)
	{
		m_gravityComp = m_Owner->GetComponent<GravityComponent>();
	}

	bool isGrounded = m_gravityComp ? m_gravityComp->IsGrounded() : true;

	DirectX::XMFLOAT3 pos = transform->GetPos();
	DirectX::XMFLOAT3 targetPos = { pos.x + movement.x, pos.y, pos.z + movement.z };

	if (m_field)
	{
		float groundY = 0.0f;
		DirectX::XMFLOAT3 normal = { 0.0f, 1.0f, 0.0f };

		if (m_field->GetHeight(targetPos.x, targetPos.z, groundY))
		{
			m_field->GetNormal(targetPos.x, targetPos.z, normal);

			// Slope limit check
			if (normal.y >= m_minClimbNormalY)
			{
				if (isGrounded)
				{
					targetPos.y = groundY;
				}
				transform->SetPos(targetPos);
			}
			else
			{
				// Steep slope -> slide along the horizontal plane
				float dot = movement.x * normal.x + movement.z * normal.z;
				if (dot < 0.0f)
				{
					float slideX = movement.x - normal.x * dot;
					float slideZ = movement.z - normal.z * dot;
					DirectX::XMFLOAT3 slidePos = { pos.x + slideX, pos.y, pos.z + slideZ };
					float slideY = 0.0f;
					if (m_field->GetHeight(slidePos.x, slidePos.z, slideY))
					{
						if (isGrounded)
						{
							slidePos.y = slideY;
						}
						transform->SetPos(slidePos);
					}
				}
			}
		}
		else
		{
			transform->SetPos(targetPos);
		}
	}
	else
	{
		transform->SetPos(targetPos);
	}

	// Auto rotate character towards movement direction
	if (m_autoRotate && (movement.x != 0.0f || movement.z != 0.0f))
	{
		float targetRotY = atan2f(movement.x, movement.z);
		transform->SetRotation({ 0.0f, targetRotY, 0.0f });
	}
}

void CharacterMovementComponent::MoveDirection(const DirectX::XMFLOAT3& direction, float deltaTime)
{
	float frameSpeed = m_speed * (deltaTime * 60.0f);
	DirectX::XMFLOAT3 movement = {
		direction.x * frameSpeed,
		0.0f,
		direction.z * frameSpeed
	};
	Move(movement, deltaTime);
}

void CharacterMovementComponent::MoveTowards(const DirectX::XMFLOAT3& targetPos, float deltaTime)
{
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	DirectX::XMFLOAT3 pos = transform->GetPos();
	float dx = targetPos.x - pos.x;
	float dz = targetPos.z - pos.z;
	float dist = sqrtf(dx * dx + dz * dz);

	if (dist > 0.001f)
	{
		DirectX::XMFLOAT3 dir = { dx / dist, 0.0f, dz / dist };
		MoveDirection(dir, deltaTime);
	}
}
