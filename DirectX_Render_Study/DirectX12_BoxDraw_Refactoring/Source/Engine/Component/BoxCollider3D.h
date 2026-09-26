#pragma once
#include "Collider3D.h"
#include <DirectXMath.h>
#include "Object.h"
#include "Transform.h"

class Camera;

class BoxCollider3D : public Collider3D
{
public:
	//コンストラクタ
	BoxCollider3D()
	{
		SetName("BoxCollider3D");
		ColliderType = Collider::BOX_3D;
	}

	//デストラクタ
	virtual ~BoxCollider3D(){}

	//WorldPosの取得
	virtual DirectX::XMFLOAT3 GetWorldPos() override
	{
		DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
		if (m_Owner)
		{
			CTransform* transform = m_Owner->GetComponent<CTransform>();
			if (transform)
			{
				pos = transform->GetPos();
			}
		}
		pos.x += Offset.x;
		pos.y += Offset.y;
		pos.z += Offset.z;
		return pos;
	}

	// Getter
	DirectX::XMFLOAT3 GetSize() const { return Size; }
	// Setter
	void SetSize(DirectX::XMFLOAT3 _Size) { Size = _Size; }

	// Calculate AABB penetration depth and minimum separation vector
	bool CalculatePenetration(const BoxCollider3D* other, DirectX::XMFLOAT3& outPushVector) const
	{
		if (!other) return false;

		DirectX::XMFLOAT3 posA = const_cast<BoxCollider3D*>(this)->GetWorldPos();
		DirectX::XMFLOAT3 sizeA = GetSize();
		DirectX::XMFLOAT3 posB = const_cast<BoxCollider3D*>(other)->GetWorldPos();
		DirectX::XMFLOAT3 sizeB = other->GetSize();

		float halfAx = sizeA.x * 0.5f;
		float halfAy = sizeA.y * 0.5f;
		float halfAz = sizeA.z * 0.5f;

		float halfBx = sizeB.x * 0.5f;
		float halfBy = sizeB.y * 0.5f;
		float halfBz = sizeB.z * 0.5f;

		float dx = posA.x - posB.x;
		float px = (halfAx + halfBx) - fabsf(dx);
		if (px <= 0.0f) return false;

		float dy = posA.y - posB.y;
		float py = (halfAy + halfBy) - fabsf(dy);
		if (py <= 0.0f) return false;

		float dz = posA.z - posB.z;
		float pz = (halfAz + halfBz) - fabsf(dz);
		if (pz <= 0.0f) return false;

		if (px < py && px < pz)
		{
			outPushVector = { (dx > 0.0f ? px : -px), 0.0f, 0.0f };
		}
		else if (pz < px && pz < py)
		{
			outPushVector = { 0.0f, 0.0f, (dz > 0.0f ? pz : -pz) };
		}
		else
		{
			outPushVector = { 0.0f, (dy > 0.0f ? py : -py), 0.0f };
		}

		return true;
	}

	// Calculate horizontal penetration (useful for character-to-character / obstacle pushing)
	bool CalculateHorizontalPenetration(const BoxCollider3D* other, DirectX::XMFLOAT3& outPushVector) const
	{
		if (!other) return false;

		DirectX::XMFLOAT3 posA = const_cast<BoxCollider3D*>(this)->GetWorldPos();
		DirectX::XMFLOAT3 sizeA = GetSize();
		DirectX::XMFLOAT3 posB = const_cast<BoxCollider3D*>(other)->GetWorldPos();
		DirectX::XMFLOAT3 sizeB = other->GetSize();

		float halfAx = sizeA.x * 0.5f;
		float halfAy = sizeA.y * 0.5f;
		float halfAz = sizeA.z * 0.5f;

		float halfBx = sizeB.x * 0.5f;
		float halfBy = sizeB.y * 0.5f;
		float halfBz = sizeB.z * 0.5f;

		float dy = posA.y - posB.y;
		float py = (halfAy + halfBy) - fabsf(dy);
		if (py <= 0.0f) return false;

		float dx = posA.x - posB.x;
		float px = (halfAx + halfBx) - fabsf(dx);
		if (px <= 0.0f) return false;

		float dz = posA.z - posB.z;
		float pz = (halfAz + halfBz) - fabsf(dz);
		if (pz <= 0.0f) return false;

		if (px < pz)
		{
			outPushVector = { (dx > 0.0f ? px : -px), 0.0f, 0.0f };
		}
		else
		{
			outPushVector = { 0.0f, 0.0f, (dz > 0.0f ? pz : -pz) };
		}

		return true;
	}

	// デバッグ可視化用
	void DrawDebug(Camera* camera);

protected:
	DirectX::XMFLOAT3 Size = { 1.0f, 1.0f, 1.0f };	//大きさ
};