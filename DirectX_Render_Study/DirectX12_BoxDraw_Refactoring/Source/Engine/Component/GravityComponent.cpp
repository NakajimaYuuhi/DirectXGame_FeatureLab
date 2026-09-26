#include "GravityComponent.h"
#include "Object.h"
#include "Transform.h"
#include "Field.h"
#include "ObjectManager.h"
#include <algorithm>

GravityComponent::GravityComponent()
	: CComponent("GravityComponent")
{
}

GravityComponent::GravityComponent(float gravity, float jumpPower)
	: CComponent("GravityComponent"), m_gravity(gravity), m_jumpPower(jumpPower)
{
}

void GravityComponent::Start()
{
	m_field = ObjectManager::GetInstance().GetField();
	SnapToGround();
}

void GravityComponent::Jump()
{
	Jump(m_jumpPower);
}

void GravityComponent::Jump(float power)
{
	if (m_isGrounded)
	{
		m_verticalVelocity = power;
		m_isGrounded = false;
	}
}

void GravityComponent::SnapToGround()
{
	if (!m_Owner) return;

	if (!m_field)
	{
		m_field = ObjectManager::GetInstance().GetField();
	}

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	DirectX::XMFLOAT3 pos = transform->GetPos();
	float groundY = 0.0f;

	if (m_field && m_field->GetHeight(pos.x, pos.z, groundY))
	{
		pos.y = groundY;
		transform->SetPos(pos);
		m_verticalVelocity = 0.0f;
		m_isGrounded = true;
	}
}

void GravityComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	if (!m_field)
	{
		m_field = ObjectManager::GetInstance().GetField();
	}

	DirectX::XMFLOAT3 pos = transform->GetPos();
	float groundY = 0.0f;
	bool hasGround = (m_field != nullptr) && m_field->GetHeight(pos.x, pos.z, groundY);

	if (!hasGround)
	{
		groundY = 0.0f;
	}

	// Apply gravity acceleration
	m_verticalVelocity += m_gravity * deltaTime;
	if (m_verticalVelocity < m_terminalVelocity)
	{
		m_verticalVelocity = m_terminalVelocity;
	}

	float nextY = pos.y + m_verticalVelocity * deltaTime;

	// Ground check
	if (m_isGrounded && m_verticalVelocity <= 0.0f)
	{
		float diff = pos.y - groundY;
		if (diff >= -0.1f && diff <= m_stepDownLimit)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else if (nextY <= groundY)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else
		{
			pos.y = nextY;
			m_isGrounded = false;
		}
	}
	else
	{
		if (nextY <= groundY)
		{
			pos.y = groundY;
			m_verticalVelocity = 0.0f;
			m_isGrounded = true;
		}
		else
		{
			pos.y = nextY;
			m_isGrounded = false;
		}
	}

	transform->SetPos(pos);
}
