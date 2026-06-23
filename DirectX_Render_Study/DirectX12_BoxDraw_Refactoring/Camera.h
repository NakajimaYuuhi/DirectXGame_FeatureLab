#pragma once
#include "3D_Object.h"
#include "StringAlias.h"

class Player;

class Camera : public C3D_Object
{
public:
	Camera(String _Name);
	~Camera() = default;

	void Init() override;
	void Update() override;

	DirectX::XMMATRIX GetView() { return view; }
	DirectX::XMMATRIX GetProj() { return proj; }
	float GetAngleY() const { return m_angleY; }

private:
	DirectX::XMMATRIX view;
	DirectX::XMMATRIX proj;

	Player* m_player = nullptr;
	float m_angleY = 0.0f;
	float m_distance = 5.0f;
	float m_height = 2.5f;
	float m_rotationSpeed = 0.02f;
};
