#pragma once
#include "Component.h"
#include <DirectXMath.h>

class CameraComponent : public CComponent
{
public:
	CameraComponent();
	virtual ~CameraComponent() = default;

	void Init() override;
	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::PostPhysics; }

	// View & Projection Matrices
	DirectX::XMMATRIX GetViewMatrix() const { return m_viewMatrix; }
	DirectX::XMMATRIX GetProjectionMatrix() const { return m_projMatrix; }
	DirectX::XMFLOAT3 GetEyePosition() const { return m_eyePos; }
	DirectX::XMFLOAT3 GetTargetPosition() const { return m_targetPos; }

	// Camera Shake Trigger
	void Shake(float intensity = 0.25f, float duration = 0.4f);

	// Getters & Setters
	void SetTarget(CObject* target) { m_target = target; }
	CObject* GetTarget() const { return m_target; }

	void SetDistance(float dist) { m_distance = dist; }
	float GetDistance() const { return m_distance; }

	void SetHeight(float height) { m_height = height; }
	float GetHeight() const { return m_height; }

	void SetAngleY(float angleY) { m_angleY = angleY; }
	float GetAngleY() const { return m_angleY; }

	void SetAngleX(float angleX) { m_angleX = angleX; }
	float GetAngleX() const { return m_angleX; }

	void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
	float GetRotationSpeed() const { return m_rotationSpeed; }

	void SetFollowSpeed(float speed) { m_followSpeed = speed; }
	float GetFollowSpeed() const { return m_followSpeed; }

	void SetTargetOffset(const DirectX::XMFLOAT3& offset) { m_targetOffset = offset; }
	DirectX::XMFLOAT3 GetTargetOffset() const { return m_targetOffset; }

	void SetFov(float fov) { m_fov = fov; UpdateProjection(); }
	float GetFov() const { return m_fov; }

	void SetNearZ(float nearZ) { m_nearZ = nearZ; UpdateProjection(); }
	float GetNearZ() const { return m_nearZ; }

	void SetFarZ(float farZ) { m_farZ = farZ; UpdateProjection(); }
	float GetFarZ() const { return m_farZ; }

	void UpdateProjection();

private:
	// Projection parameters
	float m_fov = DirectX::XM_PIDIV4; // 45 degrees
	float m_nearZ = 0.1f;
	float m_farZ = 1000.0f;
	float m_aspectRatio = 16.0f / 9.0f;

	// Orbit / Follow parameters
	CObject* m_target = nullptr;
	float m_distance = 5.0f;
	float m_height = 2.5f;
	float m_angleY = 0.0f;
	float m_angleX = 0.0f;
	float m_rotationSpeed = 0.02f;
	float m_followSpeed = 12.0f; // Smooth lerp speed
	DirectX::XMFLOAT3 m_targetOffset = { 0.0f, 1.0f, 0.0f };

	// Current smooth position & matrices
	DirectX::XMFLOAT3 m_eyePos = { 0.0f, 2.5f, -5.0f };
	DirectX::XMFLOAT3 m_targetPos = { 0.0f, 1.0f, 0.0f };
	DirectX::XMMATRIX m_viewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX m_projMatrix = DirectX::XMMatrixIdentity();

	// Camera Shake state
	float m_shakeIntensity = 0.0f;
	float m_shakeDuration = 0.0f;
	float m_shakeTimer = 0.0f;
};
