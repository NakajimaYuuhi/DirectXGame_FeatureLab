#pragma once
#include "3D_Object.h"
#include "StringAlias.h"
#include "CameraComponent.h"

class Camera : public C3D_Object
{
public:
	Camera(String _Name);
	~Camera() = default;

	void Init() override;
	void Awake() override;
	void Start() override;
	void Update() override;

	DirectX::XMMATRIX GetView();
	DirectX::XMMATRIX GetProj();
	float GetAngleY() const;

	CameraComponent* GetCameraComponent() const { return m_cameraComponent; }

private:
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

	CameraComponent* m_cameraComponent = nullptr;
	CObject* m_player = nullptr;
	float m_angleY = 0.0f;
	float m_distance = 5.0f;
	float m_height = 2.5f;
	float m_rotationSpeed = 0.02f;
};
