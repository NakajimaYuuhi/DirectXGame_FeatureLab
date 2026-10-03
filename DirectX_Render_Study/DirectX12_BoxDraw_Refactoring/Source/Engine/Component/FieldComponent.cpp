#include "FieldComponent.h"
#include "Object.h"
#include "Model.h"
#include "Transform.h"
#include <cmath>
#include <algorithm>

FieldComponent::FieldComponent()
	: CComponent("FieldComponent")
{
}

void FieldComponent::Init()
{
	Awake();
}

void FieldComponent::Awake()
{
	GenerateTerrainMesh();
}

void FieldComponent::Start()
{
	GenerateTerrainMesh();
}

void FieldComponent::Update(float deltaTime)
{
	if (!m_isMeshGenerated)
	{
		GenerateTerrainMesh();
	}
}

float FieldComponent::CalculateProceduralHeight(float x, float z)
{
	float distSq = x * x + z * z;
	float flattenFactor = 1.0f - std::exp(-distSq / 80.0f);

	float wave1 = sinf(x * 0.12f) * cosf(z * 0.12f) * 2.2f;
	float wave2 = sinf(x * 0.28f + 1.2f) * sinf(z * 0.25f + 0.8f) * 0.8f;
	float wave3 = cosf(x * 0.06f) * sinf(z * 0.06f) * 1.5f;

	return (wave1 + wave2 + wave3) * flattenFactor;
}

void FieldComponent::GenerateTerrainMesh()
{
	if (m_isMeshGenerated || !m_Owner) return;

	CModel* model = m_Owner->GetComponent<CModel>();
	if (!model) model = m_Owner->AddComponent<CModel>();
	if (!model) return;

	int numVertsX = m_gridX + 1;
	int numVertsZ = m_gridZ + 1;
	size_t totalVertices = static_cast<size_t>(numVertsX * numVertsZ);

	std::vector<MeshVertex> vertices(totalVertices);
	m_heightMap.resize(totalVertices);

	float halfW = m_width * 0.5f;
	float halfD = m_depth * 0.5f;
	float dx = m_width / static_cast<float>(m_gridX);
	float dz = m_depth / static_cast<float>(m_gridZ);

	for (int z = 0; z <= m_gridZ; ++z)
	{
		for (int x = 0; x <= m_gridX; ++x)
		{
			int idx = z * numVertsX + x;

			float posX = -halfW + x * dx;
			float posZ = -halfD + z * dz;
			float posY = CalculateProceduralHeight(posX, posZ);

			m_heightMap[idx] = posY;

			vertices[idx].position[0] = posX;
			vertices[idx].position[1] = posY;
			vertices[idx].position[2] = posZ;

			vertices[idx].uv[0] = (static_cast<float>(x) / static_cast<float>(m_gridX)) * m_uvTiling;
			vertices[idx].uv[1] = (static_cast<float>(z) / static_cast<float>(m_gridZ)) * m_uvTiling;

			vertices[idx].boneIndices[0] = 0;
			vertices[idx].boneIndices[1] = 0;
			vertices[idx].boneIndices[2] = 0;
			vertices[idx].boneIndices[3] = 0;
			vertices[idx].boneWeights[0] = 1.0f;
			vertices[idx].boneWeights[1] = 0.0f;
			vertices[idx].boneWeights[2] = 0.0f;
			vertices[idx].boneWeights[3] = 0.0f;
		}
	}

	for (int z = 0; z <= m_gridZ; ++z)
	{
		for (int x = 0; x <= m_gridX; ++x)
		{
			int idx = z * numVertsX + x;

			float hL = (x > 0) ? m_heightMap[z * numVertsX + (x - 1)] : m_heightMap[idx];
			float hR = (x < m_gridX) ? m_heightMap[z * numVertsX + (x + 1)] : m_heightMap[idx];
			float hD = (z > 0) ? m_heightMap[(z - 1) * numVertsX + x] : m_heightMap[idx];
			float hU = (z < m_gridZ) ? m_heightMap[(z + 1) * numVertsX + x] : m_heightMap[idx];

			float stepX = (x > 0 && x < m_gridX) ? (2.0f * dx) : dx;
			float stepZ = (z > 0 && z < m_gridZ) ? (2.0f * dz) : dz;

			DirectX::XMFLOAT3 normal = {
				-(hR - hL) / stepX,
				1.0f,
				-(hU - hD) / stepZ
			};

			float len = std::sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			if (len > 0.0f)
			{
				normal.x /= len;
				normal.y /= len;
				normal.z /= len;
			}

			vertices[idx].normal[0] = normal.x;
			vertices[idx].normal[1] = normal.y;
			vertices[idx].normal[2] = normal.z;
		}
	}

	std::vector<uint32_t> indices;
	indices.reserve(static_cast<size_t>(m_gridX * m_gridZ * 6));

	for (int z = 0; z < m_gridZ; ++z)
	{
		for (int x = 0; x < m_gridX; ++x)
		{
			uint32_t v0 = static_cast<uint32_t>(z * numVertsX + x);
			uint32_t v1 = static_cast<uint32_t>(z * numVertsX + (x + 1));
			uint32_t v2 = static_cast<uint32_t>((z + 1) * numVertsX + x);
			uint32_t v3 = static_cast<uint32_t>((z + 1) * numVertsX + (x + 1));

			indices.push_back(v0);
			indices.push_back(v2);
			indices.push_back(v1);

			indices.push_back(v1);
			indices.push_back(v2);
			indices.push_back(v3);
		}
	}

	UINT matIdx = model->RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 0.45f, 0.75f, 0.4f, 1.0f });
	model->RegisterMesh(matIdx, vertices.data(), vertices.size(), indices.data(), indices.size());
	model->CreateBoneBuffer();

	m_isMeshGenerated = true;
}

bool FieldComponent::GetHeight(float worldX, float worldZ, float& outHeight) const
{
	if (!m_Owner) return false;

	CTransform* trans = m_Owner->GetComponent<CTransform>();
	DirectX::XMFLOAT3 fPos = trans ? trans->GetPos() : DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 fScale = trans ? trans->GetScale() : DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };

	if (fScale.x == 0.0f || fScale.z == 0.0f)
	{
		outHeight = fPos.y;
		return true;
	}

	float localX = (worldX - fPos.x) / fScale.x;
	float localZ = (worldZ - fPos.z) / fScale.z;

	float halfW = m_width * 0.5f;
	float halfD = m_depth * 0.5f;

	if (localX < -halfW || localX > halfW || localZ < -halfD || localZ > halfD)
	{
		outHeight = fPos.y;
		return false;
	}

	float dx = m_width / static_cast<float>(m_gridX);
	float dz = m_depth / static_cast<float>(m_gridZ);

	float fx = (localX + halfW) / dx;
	float fz = (localZ + halfD) / dz;

	int cellX = static_cast<int>(std::floor(fx));
	int cellZ = static_cast<int>(std::floor(fz));

	if (cellX < 0) cellX = 0;
	if (cellX >= m_gridX) cellX = m_gridX - 1;
	if (cellZ < 0) cellZ = 0;
	if (cellZ >= m_gridZ) cellZ = m_gridZ - 1;

	float u = fx - static_cast<float>(cellX);
	float v = fz - static_cast<float>(cellZ);

	int numVertsX = m_gridX + 1;
	int v0 = cellZ * numVertsX + cellX;
	int v1 = cellZ * numVertsX + (cellX + 1);
	int v2 = (cellZ + 1) * numVertsX + cellX;
	int v3 = (cellZ + 1) * numVertsX + (cellX + 1);

	if (m_heightMap.empty())
	{
		outHeight = fPos.y;
		return false;
	}

	float h0 = m_heightMap[v0];
	float h1 = m_heightMap[v1];
	float h2 = m_heightMap[v2];
	float h3 = m_heightMap[v3];

	float localHeight = 0.0f;

	if (u + v <= 1.0f)
	{
		localHeight = h0 + u * (h1 - h0) + v * (h2 - h0);
	}
	else
	{
		localHeight = h3 + (1.0f - u) * (h2 - h3) + (1.0f - v) * (h1 - h3);
	}

	outHeight = fPos.y + localHeight * fScale.y;
	return true;
}

bool FieldComponent::GetNormal(float worldX, float worldZ, DirectX::XMFLOAT3& outNormal) const
{
	if (!m_Owner) return false;

	CTransform* trans = m_Owner->GetComponent<CTransform>();
	DirectX::XMFLOAT3 fPos = trans ? trans->GetPos() : DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 fScale = trans ? trans->GetScale() : DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };

	if (fScale.x == 0.0f || fScale.z == 0.0f)
	{
		outNormal = { 0.0f, 1.0f, 0.0f };
		return false;
	}

	float localX = (worldX - fPos.x) / fScale.x;
	float localZ = (worldZ - fPos.z) / fScale.z;

	float halfW = m_width * 0.5f;
	float halfD = m_depth * 0.5f;

	if (localX < -halfW || localX > halfW || localZ < -halfD || localZ > halfD)
	{
		outNormal = { 0.0f, 1.0f, 0.0f };
		return false;
	}

	float dx = m_width / static_cast<float>(m_gridX);
	float dz = m_depth / static_cast<float>(m_gridZ);

	float fx = (localX + halfW) / dx;
	float fz = (localZ + halfD) / dz;

	int cellX = static_cast<int>(std::floor(fx));
	int cellZ = static_cast<int>(std::floor(fz));

	if (cellX < 0) cellX = 0;
	if (cellX >= m_gridX) cellX = m_gridX - 1;
	if (cellZ < 0) cellZ = 0;
	if (cellZ >= m_gridZ) cellZ = m_gridZ - 1;

	float u = fx - static_cast<float>(cellX);
	float v = fz - static_cast<float>(cellZ);

	int numVertsX = m_gridX + 1;
	int v0 = cellZ * numVertsX + cellX;
	int v1 = cellZ * numVertsX + (cellX + 1);
	int v2 = (cellZ + 1) * numVertsX + cellX;
	int v3 = (cellZ + 1) * numVertsX + (cellX + 1);

	if (m_heightMap.empty())
	{
		outNormal = { 0.0f, 1.0f, 0.0f };
		return false;
	}

	float h0 = m_heightMap[v0];
	float h1 = m_heightMap[v1];
	float h2 = m_heightMap[v2];
	float h3 = m_heightMap[v3];

	DirectX::XMFLOAT3 n;
	if (u + v <= 1.0f)
	{
		n.x = -(h1 - h0) * dz;
		n.y = dx * dz;
		n.z = -(h2 - h0) * dx;
	}
	else
	{
		n.x = -(h3 - h2) * dz;
		n.y = dx * dz;
		n.z = -(h3 - h1) * dx;
	}

	float len = std::sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
	if (len > 0.0f)
	{
		n.x /= len;
		n.y /= len;
		n.z /= len;
	}
	else
	{
		n = { 0.0f, 1.0f, 0.0f };
	}

	outNormal = n;
	return true;
}
