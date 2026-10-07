#include "CapsuleCollider3D.h"
#include "Object.h"
#include "Transform.h"
#include "CameraComponent.h"
#include "imgui.h"
#include <cmath>
#include <vector>

CapsuleCollider3D::CapsuleCollider3D()
{
	SetName("CapsuleCollider3D");
	ColliderType = Collider::CAPSULE_3D;
}

DirectX::XMFLOAT3 CapsuleCollider3D::GetWorldPos()
{
	DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
	if (m_Owner)
	{
		CTransform* transform = m_Owner->GetComponent<CTransform>();
		if (transform)
		{
			pos = transform->GetWorldPosition();
		}
	}
	pos.x += Offset.x;
	pos.y += Offset.y;
	pos.z += Offset.z;
	return pos;
}

void CapsuleCollider3D::GetSegmentEndpoints(DirectX::XMFLOAT3& outBottom, DirectX::XMFLOAT3& outTop) const
{
	DirectX::XMFLOAT3 center = const_cast<CapsuleCollider3D*>(this)->GetWorldPos();
	float cylinderHeight = (std::max)(0.0f, m_height - 2.0f * m_radius);
	float halfCylinder = cylinderHeight * 0.5f;

	outBottom = { center.x, center.y - halfCylinder, center.z };
	outTop    = { center.x, center.y + halfCylinder, center.z };
}

void CapsuleCollider3D::DrawDebug(CameraComponent* camera, ImDrawList* customDrawList, const ImVec2& vpPos, const ImVec2& vpSize)
{
#ifndef _DEBUG
	return;
#endif // !_DEBUG

	if (!camera) return;

	DirectX::XMFLOAT3 bottomCenter, topCenter;
	GetSegmentEndpoints(bottomCenter, topCenter);

	DirectX::XMMATRIX viewProj = camera->GetViewMatrix() * camera->GetProjectionMatrix();
	ImGuiIO& io = ImGui::GetIO();

	bool useViewport = (vpSize.x > 0.0f && vpSize.y > 0.0f);
	float screenW = useViewport ? vpSize.x : io.DisplaySize.x;
	float screenH = useViewport ? vpSize.y : io.DisplaySize.y;
	float offsetX = useViewport ? vpPos.x : 0.0f;
	float offsetY = useViewport ? vpPos.y : 0.0f;

	auto WorldToScreen = [&](const DirectX::XMFLOAT3& worldPos, ImVec2& outScreen) -> bool
	{
		DirectX::XMVECTOR v = DirectX::XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);
		DirectX::XMVECTOR clip = DirectX::XMVector4Transform(v, viewProj);
		float w = DirectX::XMVectorGetW(clip);
		if (w <= 0.1f) return false;

		float x = DirectX::XMVectorGetX(clip) / w;
		float y = DirectX::XMVectorGetY(clip) / w;
		outScreen.x = offsetX + (x + 1.0f) * 0.5f * screenW;
		outScreen.y = offsetY + (1.0f - y) * 0.5f * screenH;
		return true;
	};

	ImDrawList* drawList = customDrawList ? customDrawList : ImGui::GetBackgroundDrawList();
	if (!drawList) return;

	if (useViewport)
	{
		drawList->PushClipRect(vpPos, ImVec2(vpPos.x + vpSize.x, vpPos.y + vpSize.y), true);
	}

	ImU32 debugColor = m_isTrigger ? IM_COL32(255, 220, 40, 255) : IM_COL32(40, 240, 100, 255);
	const float kPi = 3.14159265358979323846f;
	const int kSegments = 16;

	auto DrawSegment3D = [&](const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& p2)
	{
		ImVec2 s1, s2;
		if (WorldToScreen(p1, s1) && WorldToScreen(p2, s2))
		{
			drawList->AddLine(s1, s2, debugColor, 2.0f);
		}
	};

	// 1. Horizontal circle at Top Center (XZ plane)
	for (int i = 0; i < kSegments; ++i)
	{
		float a1 = (float)i * 2.0f * kPi / (float)kSegments;
		float a2 = (float)(i + 1) * 2.0f * kPi / (float)kSegments;
		DirectX::XMFLOAT3 p1 = { topCenter.x + m_radius * cosf(a1), topCenter.y, topCenter.z + m_radius * sinf(a1) };
		DirectX::XMFLOAT3 p2 = { topCenter.x + m_radius * cosf(a2), topCenter.y, topCenter.z + m_radius * sinf(a2) };
		DrawSegment3D(p1, p2);
	}

	// 2. Horizontal circle at Bottom Center (XZ plane)
	for (int i = 0; i < kSegments; ++i)
	{
		float a1 = (float)i * 2.0f * kPi / (float)kSegments;
		float a2 = (float)(i + 1) * 2.0f * kPi / (float)kSegments;
		DirectX::XMFLOAT3 p1 = { bottomCenter.x + m_radius * cosf(a1), bottomCenter.y, bottomCenter.z + m_radius * sinf(a1) };
		DirectX::XMFLOAT3 p2 = { bottomCenter.x + m_radius * cosf(a2), bottomCenter.y, bottomCenter.z + m_radius * sinf(a2) };
		DrawSegment3D(p1, p2);
	}

	// 3. Cylinder 4 vertical side lines
	DrawSegment3D({ topCenter.x + m_radius, topCenter.y, topCenter.z }, { bottomCenter.x + m_radius, bottomCenter.y, bottomCenter.z });
	DrawSegment3D({ topCenter.x - m_radius, topCenter.y, topCenter.z }, { bottomCenter.x - m_radius, bottomCenter.y, bottomCenter.z });
	DrawSegment3D({ topCenter.x, topCenter.y, topCenter.z + m_radius }, { bottomCenter.x, bottomCenter.y, bottomCenter.z + m_radius });
	DrawSegment3D({ topCenter.x, topCenter.y, topCenter.z - m_radius }, { bottomCenter.x, bottomCenter.y, bottomCenter.z - m_radius });

	// 4. Top Dome Arcs (XY and ZY vertical semicircles)
	const int kHalfSegments = 8;
	for (int i = 0; i < kHalfSegments; ++i)
	{
		float a1 = (float)i * kPi / (float)kHalfSegments;
		float a2 = (float)(i + 1) * kPi / (float)kHalfSegments;

		// XY arc
		DirectX::XMFLOAT3 xy1 = { topCenter.x + m_radius * cosf(a1), topCenter.y + m_radius * sinf(a1), topCenter.z };
		DirectX::XMFLOAT3 xy2 = { topCenter.x + m_radius * cosf(a2), topCenter.y + m_radius * sinf(a2), topCenter.z };
		DrawSegment3D(xy1, xy2);

		// ZY arc
		DirectX::XMFLOAT3 zy1 = { topCenter.x, topCenter.y + m_radius * sinf(a1), topCenter.z + m_radius * cosf(a1) };
		DirectX::XMFLOAT3 zy2 = { topCenter.x, topCenter.y + m_radius * sinf(a2), topCenter.z + m_radius * cosf(a2) };
		DrawSegment3D(zy1, zy2);
	}

	// 5. Bottom Dome Arcs (XY and ZY vertical downward semicircles)
	for (int i = 0; i < kHalfSegments; ++i)
	{
		float a1 = (float)i * kPi / (float)kHalfSegments;
		float a2 = (float)(i + 1) * kPi / (float)kHalfSegments;

		// XY downward arc
		DirectX::XMFLOAT3 xy1 = { bottomCenter.x + m_radius * cosf(a1), bottomCenter.y - m_radius * sinf(a1), bottomCenter.z };
		DirectX::XMFLOAT3 xy2 = { bottomCenter.x + m_radius * cosf(a2), bottomCenter.y - m_radius * sinf(a2), bottomCenter.z };
		DrawSegment3D(xy1, xy2);

		// ZY downward arc
		DirectX::XMFLOAT3 zy1 = { bottomCenter.x, bottomCenter.y - m_radius * sinf(a1), bottomCenter.z + m_radius * cosf(a1) };
		DirectX::XMFLOAT3 zy2 = { bottomCenter.x, bottomCenter.y - m_radius * sinf(a2), bottomCenter.z + m_radius * cosf(a2) };
		DrawSegment3D(zy1, zy2);
	}

	if (useViewport)
	{
		drawList->PopClipRect();
	}
}
