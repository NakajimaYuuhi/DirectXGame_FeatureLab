#pragma once
#include "3D_Object.h"
#include <vector>
#include <DirectXMath.h>

class Field : public C3D_Object
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
