#pragma once
#include "Object.h"
#include "Transform.h"
#include <vector>
#include <DirectXMath.h>

class Field : public CObject
{
public:
	Field(String _Name = "Field");
	virtual ~Field() = default;

	void Init() override;
	void Awake() override;
	void Start() override;
	void Update() override;

	// Ground height collision test
	bool GetHeight(float worldX, float worldZ, float& outHeight) const;

	// Ground normal calculation
	bool GetNormal(float worldX, float worldZ, DirectX::XMFLOAT3& outNormal) const;

	// Field parameters
	float GetWidth() const { return m_width; }
	float GetDepth() const { return m_depth; }
	int GetGridX() const { return m_gridX; }
	int GetGridZ() const { return m_gridZ; }

	// Procedural height formula
	static float CalculateProceduralHeight(float x, float z);

	// Transform wrappers for compatibility
	DirectX::XMFLOAT3 GetPos() {
		if (auto t = GetComponent<CTransform>()) return t->GetPos();
		return { 0.0f, 0.0f, 0.0f };
	}
	void SetPos(const DirectX::XMFLOAT3& pos) {
		if (auto t = GetComponent<CTransform>()) t->SetPos(pos);
	}
	DirectX::XMFLOAT3 GetScale() {
		if (auto t = GetComponent<CTransform>()) return t->GetScale();
		return { 1.0f, 1.0f, 1.0f };
	}
	void SetScale(const DirectX::XMFLOAT3& scale) {
		if (auto t = GetComponent<CTransform>()) t->SetScale(scale);
	}
	DirectX::XMFLOAT3 GetRotation() {
		if (auto t = GetComponent<CTransform>()) return t->GetRotation();
		return { 0.0f, 0.0f, 0.0f };
	}
	void SetRotation(const DirectX::XMFLOAT3& rot) {
		if (auto t = GetComponent<CTransform>()) t->SetRotation(rot);
	}
	DirectX::XMFLOAT3 GetFront() {
		if (auto t = GetComponent<CTransform>()) return t->GetFront();
		return { 0.0f, 0.0f, 1.0f };
	}
	DirectX::XMFLOAT3 GetRight() {
		if (auto t = GetComponent<CTransform>()) return t->GetRight();
		return { 1.0f, 0.0f, 0.0f };
	}
	DirectX::XMFLOAT3 GetUp() {
		if (auto t = GetComponent<CTransform>()) return t->GetUp();
		return { 0.0f, 1.0f, 0.0f };
	}

private:
	void GenerateTerrainMesh();

private:
	float m_width = 80.0f;
	float m_depth = 80.0f;
	int m_gridX = 80;
	int m_gridZ = 80;
	float m_uvTiling = 20.0f;

	std::vector<float> m_heightMap;

	bool m_isMeshGenerated = false;
};
