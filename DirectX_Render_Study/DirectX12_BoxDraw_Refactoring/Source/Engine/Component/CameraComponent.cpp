#include "CameraComponent.h"
#include "Object.h"
#include "Transform.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "BasicSettings.h"
#include <cmath>
#include <cstdlib>

CameraComponent::CameraComponent()
	: CComponent("CameraComponent")
{
	m_aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
	UpdateProjection();
}

void CameraComponent::Init()
{
	UpdateProjection();
}

void CameraComponent::Start()
{
	if (!m_target || m_target->GetIsDestroyed())
	{
		m_target = ObjectManager::GetInstance().GetPlayer();
	}
	UpdateProjection();

	// Snap position immediately to target on start
	if (m_target)
	{
		CTransform* targetTransform = m_target->GetComponent<CTransform>();
		if (targetTransform)
		{
			DirectX::XMFLOAT3 pos = targetTransform->GetPos();
			m_targetPos = { pos.x + m_targetOffset.x, pos.y + m_targetOffset.y, pos.z + m_targetOffset.z };

			float cosPitch = cosf(m_angleX);
			float sinPitch = sinf(m_angleX);
			float cosYaw = cosf(m_angleY);
			float sinYaw = sinf(m_angleY);

			float offsetX = sinYaw * cosPitch * m_distance;
			float offsetY = m_height + sinPitch * m_distance;
			float offsetZ = -cosYaw * cosPitch * m_distance;

			m_eyePos = { m_targetPos.x + offsetX, m_targetPos.y + offsetY, m_targetPos.z + offsetZ };

			DirectX::XMVECTOR eye = DirectX::XMVectorSet(m_eyePos.x, m_eyePos.y, m_eyePos.z, 1.0f);
			DirectX::XMVECTOR target = DirectX::XMVectorSet(m_targetPos.x, m_targetPos.y, m_targetPos.z, 1.0f);
			DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			m_viewMatrix = DirectX::XMMatrixLookAtLH(eye, target, up);
		}
	}
}

void CameraComponent::UpdateProjection()
{
	m_projMatrix = DirectX::XMMatrixPerspectiveFovLH(
		m_fov,
		m_aspectRatio,
		m_nearZ,
		m_farZ
	);
}

void CameraComponent::Shake(float intensity, float duration)
{
	m_shakeIntensity = intensity;
	m_shakeDuration = duration;
	m_shakeTimer = duration;
}

void CameraComponent::Update(float deltaTime)
{
	// Input for camera rotation (L / J keys)
	if (CInputManager::GetInstance().IsKeyPress('L'))
	{
		m_angleY -= m_rotationSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('J'))
	{
		m_angleY += m_rotationSpeed;
	}

	// Auto-find target if null or destroyed
	if (!m_target || m_target->GetIsDestroyed())
	{
		m_target = ObjectManager::GetInstance().GetPlayer();
	}

	DirectX::XMFLOAT3 desiredTargetPos = { 0.0f, 1.0f, 0.0f };
	if (m_target)
	{
		CTransform* targetTransform = m_target->GetComponent<CTransform>();
		if (targetTransform)
		{
			DirectX::XMFLOAT3 pos = targetTransform->GetPos();
			desiredTargetPos.x = pos.x + m_targetOffset.x;
			desiredTargetPos.y = pos.y + m_targetOffset.y;
			desiredTargetPos.z = pos.z + m_targetOffset.z;
		}
	}

	// Calculate offset based on yaw angleY and pitch angleX
	float cosPitch = cosf(m_angleX);
	float sinPitch = sinf(m_angleX);
	float cosYaw = cosf(m_angleY);
	float sinYaw = sinf(m_angleY);

	float offsetX = sinYaw * cosPitch * m_distance;
	float offsetY = m_height + sinPitch * m_distance;
	float offsetZ = -cosYaw * cosPitch * m_distance;

	DirectX::XMFLOAT3 desiredEyePos = {
		desiredTargetPos.x + offsetX,
		desiredTargetPos.y + offsetY,
		desiredTargetPos.z + offsetZ
	};

	// Smooth Lerp Follow
	if (m_followSpeed > 0.0f && deltaTime > 0.0f)
	{
		float factor = 1.0f - expf(-m_followSpeed * deltaTime);
		m_eyePos.x += (desiredEyePos.x - m_eyePos.x) * factor;
		m_eyePos.y += (desiredEyePos.y - m_eyePos.y) * factor;
		m_eyePos.z += (desiredEyePos.z - m_eyePos.z) * factor;

		m_targetPos.x += (desiredTargetPos.x - m_targetPos.x) * factor;
		m_targetPos.y += (desiredTargetPos.y - m_targetPos.y) * factor;
		m_targetPos.z += (desiredTargetPos.z - m_targetPos.z) * factor;
	}
	else
	{
		m_eyePos = desiredEyePos;
		m_targetPos = desiredTargetPos;
	}

	// Apply Camera Shake Jitter
	DirectX::XMFLOAT3 finalEyePos = m_eyePos;
	DirectX::XMFLOAT3 finalTargetPos = m_targetPos;

	if (m_shakeTimer > 0.0f)
	{
		m_shakeTimer -= deltaTime;
		float shakeFactor = (m_shakeDuration > 0.0f) ? (m_shakeTimer / m_shakeDuration) : 0.0f;
		float currentIntensity = m_shakeIntensity * shakeFactor;

		float randX = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * currentIntensity;
		float randY = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * currentIntensity;
		float randZ = (((float)rand() / RAND_MAX) * 2.0f - 1.0f) * currentIntensity;

		finalEyePos.x += randX;
		finalEyePos.y += randY;
		finalEyePos.z += randZ;

		finalTargetPos.x += randX;
		finalTargetPos.y += randY;
		finalTargetPos.z += randZ;
	}

	// Compute View Matrix
	DirectX::XMVECTOR eye = DirectX::XMVectorSet(finalEyePos.x, finalEyePos.y, finalEyePos.z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorSet(finalTargetPos.x, finalTargetPos.y, finalTargetPos.z, 1.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	m_viewMatrix = DirectX::XMMatrixLookAtLH(eye, target, up);
}
