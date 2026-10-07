#pragma once
#include "Collider3D.h"
#include <DirectXMath.h>
#include <algorithm>

#include "imgui.h"

class CameraComponent;
class BoxCollider3D;

class CapsuleCollider3D : public Collider3D
{
public:
	CapsuleCollider3D();
	virtual ~CapsuleCollider3D() = default;

	// World Center Position (Transform Pos + Offset)
	virtual DirectX::XMFLOAT3 GetWorldPos() override;

	// Radius & Height Accessors
	float GetRadius() const { return m_radius; }
	void SetRadius(float radius) { m_radius = (std::max)(0.01f, radius); }

	float GetHeight() const { return m_height; }
	void SetHeight(float height) { m_height = (std::max)(m_radius * 2.0f, height); }

	// Get segment endpoints (Bottom sphere center & Top sphere center)
	void GetSegmentEndpoints(DirectX::XMFLOAT3& outBottom, DirectX::XMFLOAT3& outTop) const;

	// Geometric helper methods
	static float ClosestPtSegmentSegment(
		const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& q1,
		const DirectX::XMFLOAT3& p2, const DirectX::XMFLOAT3& q2,
		DirectX::XMFLOAT3& outC1, DirectX::XMFLOAT3& outC2);

	static float ClosestPtSegmentAABB(
		const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1,
		const DirectX::XMFLOAT3& boxMin, const DirectX::XMFLOAT3& boxMax,
		DirectX::XMFLOAT3& outClosestSegPt, DirectX::XMFLOAT3& outClosestBoxPt);

	// Intersection checks
	bool CheckCollision(const CapsuleCollider3D* other) const;
	bool CheckCollision(const BoxCollider3D* other) const;

	// Debug wireframe rendering
	void DrawDebug(CameraComponent* camera, ImDrawList* customDrawList = nullptr, const ImVec2& vpPos = ImVec2(0.0f, 0.0f), const ImVec2& vpSize = ImVec2(0.0f, 0.0f));

	RenderLayer GetRenderLayer() const override { return RenderLayer::Debug; }

private:
	float m_radius = 0.5f;
	float m_height = 2.0f;
};
