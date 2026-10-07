#include "CapsuleCollider3D.h"
#include "BoxCollider3D.h"
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

float CapsuleCollider3D::ClosestPtSegmentSegment(
	const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& q1,
	const DirectX::XMFLOAT3& p2, const DirectX::XMFLOAT3& q2,
	DirectX::XMFLOAT3& outC1, DirectX::XMFLOAT3& outC2)
{
	DirectX::XMFLOAT3 d1 = { q1.x - p1.x, q1.y - p1.y, q1.z - p1.z };
	DirectX::XMFLOAT3 d2 = { q2.x - p2.x, q2.y - p2.y, q2.z - p2.z };
	DirectX::XMFLOAT3 r  = { p1.x - p2.x, p1.y - p2.y, p1.z - p2.z };

	float a = d1.x * d1.x + d1.y * d1.y + d1.z * d1.z;
	float e = d2.x * d2.x + d2.y * d2.y + d2.z * d2.z;
	float f = d2.x * r.x + d2.y * r.y + d2.z * r.z;

	const float kEps = 1e-6f;
	float s = 0.0f;
	float t = 0.0f;

	if (a <= kEps && e <= kEps)
	{
		outC1 = p1;
		outC2 = p2;
		float dx = outC1.x - outC2.x, dy = outC1.y - outC2.y, dz = outC1.z - outC2.z;
		return dx * dx + dy * dy + dz * dz;
	}

	if (a <= kEps)
	{
		s = 0.0f;
		t = std::clamp(f / e, 0.0f, 1.0f);
	}
	else
	{
		float c = d1.x * r.x + d1.y * r.y + d1.z * r.z;
		if (e <= kEps)
		{
			t = 0.0f;
			s = std::clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			float b = d1.x * d2.x + d1.y * d2.y + d1.z * d2.z;
			float denom = a * e - b * b;

			if (denom > kEps)
			{
				s = std::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			}
			else
			{
				s = 0.0f;
			}

			t = (b * s + f) / e;

			if (t < 0.0f)
			{
				t = 0.0f;
				s = std::clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > 1.0f)
			{
				t = 1.0f;
				s = std::clamp((b - c) / a, 0.0f, 1.0f);
			}
		}
	}

	outC1 = { p1.x + d1.x * s, p1.y + d1.y * s, p1.z + d1.z * s };
	outC2 = { p2.x + d2.x * t, p2.y + d2.y * t, p2.z + d2.z * t };

	float dx = outC1.x - outC2.x;
	float dy = outC1.y - outC2.y;
	float dz = outC1.z - outC2.z;
	return dx * dx + dy * dy + dz * dz;
}

float CapsuleCollider3D::ClosestPtSegmentAABB(
	const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1,
	const DirectX::XMFLOAT3& boxMin, const DirectX::XMFLOAT3& boxMax,
	DirectX::XMFLOAT3& outClosestSegPt, DirectX::XMFLOAT3& outClosestBoxPt)
{
	auto DistSqAt = [&](float t, DirectX::XMFLOAT3& segPt, DirectX::XMFLOAT3& boxPt) -> float
	{
		segPt = {
			p0.x + t * (p1.x - p0.x),
			p0.y + t * (p1.y - p0.y),
			p0.z + t * (p1.z - p0.z)
		};
		boxPt = {
			(std::max)(boxMin.x, (std::min)(segPt.x, boxMax.x)),
			(std::max)(boxMin.y, (std::min)(segPt.y, boxMax.y)),
			(std::max)(boxMin.z, (std::min)(segPt.z, boxMax.z))
		};
		float dx = segPt.x - boxPt.x;
		float dy = segPt.y - boxPt.y;
		float dz = segPt.z - boxPt.z;
		return dx * dx + dy * dy + dz * dz;
	};

	float low = 0.0f;
	float high = 1.0f;
	DirectX::XMFLOAT3 s1, b1, s2, b2;

	for (int iter = 0; iter < 16; ++iter)
	{
		float m1 = low + (high - low) / 3.0f;
		float m2 = high - (high - low) / 3.0f;

		float d1 = DistSqAt(m1, s1, b1);
		float d2 = DistSqAt(m2, s2, b2);

		if (d1 < d2)
		{
			high = m2;
		}
		else
		{
			low = m1;
		}
	}

	float bestT = (low + high) * 0.5f;
	return DistSqAt(bestT, outClosestSegPt, outClosestBoxPt);
}

bool CapsuleCollider3D::CheckCollision(const CapsuleCollider3D* other) const
{
	if (!other) return false;

	DirectX::XMFLOAT3 aBottom, aTop;
	GetSegmentEndpoints(aBottom, aTop);

	DirectX::XMFLOAT3 bBottom, bTop;
	other->GetSegmentEndpoints(bBottom, bTop);

	DirectX::XMFLOAT3 c1, c2;
	float distSq = ClosestPtSegmentSegment(aBottom, aTop, bBottom, bTop, c1, c2);

	float radiusSum = m_radius + other->m_radius;
	return distSq <= (radiusSum * radiusSum);
}

bool CapsuleCollider3D::CheckCollision(const BoxCollider3D* other) const
{
	if (!other) return false;

	DirectX::XMFLOAT3 aBottom, aTop;
	GetSegmentEndpoints(aBottom, aTop);

	DirectX::XMFLOAT3 boxPos = const_cast<BoxCollider3D*>(other)->GetWorldPos();
	DirectX::XMFLOAT3 boxSize = other->GetSize();

	DirectX::XMFLOAT3 boxMin = {
		boxPos.x - boxSize.x * 0.5f,
		boxPos.y - boxSize.y * 0.5f,
		boxPos.z - boxSize.z * 0.5f
	};
	DirectX::XMFLOAT3 boxMax = {
		boxPos.x + boxSize.x * 0.5f,
		boxPos.y + boxSize.y * 0.5f,
		boxPos.z + boxSize.z * 0.5f
	};

	DirectX::XMFLOAT3 cSeg, cBox;
	float distSq = ClosestPtSegmentAABB(aBottom, aTop, boxMin, boxMax, cSeg, cBox);

	return distSq <= (m_radius * m_radius);
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
