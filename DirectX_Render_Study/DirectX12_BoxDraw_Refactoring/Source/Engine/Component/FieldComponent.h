#pragma once
#include "Component.h"
#include <vector>
#include <DirectXMath.h>

class FieldComponent : public CComponent
{
public:
	FieldComponent();
	~FieldComponent() override = default;

	void Init() override;
	void Awake() override;
	void Start() override;
	void Update(float deltaTime) override;

	// Ground height collision test
	bool GetHeight(float worldX, float worldZ, float& outHeight) const;

	// Ground normal calculation
	bool GetNormal(float worldX, float worldZ, DirectX::XMFLOAT3& outNormal) const;

	// Field parameters
	float GetWidth() const { return m_width; }
	void SetWidth(float w) { m_width = w; m_isMeshGenerated = false; }

	float GetDepth() const { return m_depth; }
	void SetDepth(float d) { m_depth = d; m_isMeshGenerated = false; }

	int GetGridX() const { return m_gridX; }
	void SetGridX(int gx) { m_gridX = gx; m_isMeshGenerated = false; }

	int GetGridZ() const { return m_gridZ; }
	void SetGridZ(int gz) { m_gridZ = gz; m_isMeshGenerated = false; }

	float GetUVTiling() const { return m_uvTiling; }
	void SetUVTiling(float t) { m_uvTiling = t; m_isMeshGenerated = false; }

	// Procedural height formula
	static float CalculateProceduralHeight(float x, float z);

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
