#include "Camera.h"
#include "Player.h"
#include "InputManager.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "BasicSettings.h"
#include <cmath>

#include "audio.h"

Camera::Camera(String _Name)
	:C3D_Object(_Name)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectTag(ObjectTag::CAMERA);

	view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(0, 2.5, -5, 1),
		DirectX::XMVectorSet(0, 2, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));

	proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		0.1f,
		1000.0f);

	Audio* audio = AddComponent<Audio>();
	audio->Load("Assets/Audio/BGM/Ska_01.wav");
	
}

void Camera::Init()
{
	m_player = ObjectManager::GetInstance().GetPlayer();

	Audio* audio = GetComponent<Audio>();
	audio->Play(true);
}

void Camera::Update()
{
	if (CInputManager::GetInstance().IsKeyPress('Q'))
	{
		m_angleY -= m_rotationSpeed;
	}
	if (CInputManager::GetInstance().IsKeyPress('E'))
	{
		m_angleY += m_rotationSpeed;
	}

	if (m_player)
	{
		DirectX::XMFLOAT3 playerPos = m_player->GetPos();

		float offsetX = sinf(m_angleY) * m_distance;
		float offsetZ = -cosf(m_angleY) * m_distance;

		DirectX::XMVECTOR camPos = DirectX::XMVectorSet(playerPos.x + offsetX, playerPos.y + m_height, playerPos.z + offsetZ, 1.0f);
		DirectX::XMVECTOR targetPos = DirectX::XMVectorSet(playerPos.x, playerPos.y + 1.0f, playerPos.z, 1.0f);
		DirectX::XMVECTOR up = DirectX::XMVectorSet(0, 1, 0, 0);

		view = DirectX::XMMatrixLookAtLH(camPos, targetPos, up);
	}
}
