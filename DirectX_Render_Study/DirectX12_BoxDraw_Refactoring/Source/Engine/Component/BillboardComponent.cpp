#include "BillboardComponent.h"
#include "Object.h"
#include "Transform.h"
#include "DX12Manager.h"
#include "ObjectManager.h"

BillboardComponent::BillboardComponent()
	: CComponent("BillboardComponent")
{
}

void BillboardComponent::LateUpdate(float deltaTime)
{
	if (!m_Owner) return;

	if (!ObjectManager::GetInstance().GetCameraObject()) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	DX12Manager& dx12Manager = DX12Manager::GetInstance();
	DirectX::XMMATRIX view = dx12Manager.GetView();

	// Transpose view matrix to get camera orientation vectors
	DirectX::XMMATRIX invView = DirectX::XMMatrixTranspose(view);

	DirectX::XMFLOAT3 camUp = { invView.r[1].m128_f32[0], invView.r[1].m128_f32[1], invView.r[1].m128_f32[2] };
	DirectX::XMFLOAT3 camFront = { invView.r[2].m128_f32[0], invView.r[2].m128_f32[1], invView.r[2].m128_f32[2] };

	DirectX::XMFLOAT3 up = { camFront.x, camFront.y, camFront.z };
	DirectX::XMFLOAT3 front = { camUp.x, camUp.y, camUp.z };

	if (m_lockYAxis)
	{
		up.y = 0.0f;
		float len = sqrtf(up.x * up.x + up.z * up.z);
		if (len > 0.0001f)
		{
			up.x /= len;
			up.z /= len;
		}
	}

	transform->SetRotationFromUpFront(up, front);
}
