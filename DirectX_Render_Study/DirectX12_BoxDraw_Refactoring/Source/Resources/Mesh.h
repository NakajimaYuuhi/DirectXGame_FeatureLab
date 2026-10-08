//Mesh.h
//???b?V????N???X

//===== ?C???N???[?h =====
#pragma once

//----- DirectX12??A -----
#include <d3d12.h>
#include <DirectXMath.h>

//----- ?X?}?[?g?|?C???^?p -----
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <vector>


//----- Texture -----
//?Y??????????A??U??????????
#include "Texture.h"
#include "BasicSettings.h"

//?\??????
#include "ModelData.h"

//===== ?O???? =====
class CObject;
class CTransform;	//???t???[???g??????u?????
class CMaterial;

//===== ?\?????`(????????) =====
struct MeshConstantBufferData
{
	DirectX::XMMATRIX WVP;
};

//===== ?N???X??` =====
class CMesh
{

//===== ?????? =====
//?r???{?[?h????p




public:
	//Initialize??????????K?v?L??
	CMesh();

	//DX12Manager????��????(??)
	void Init();
	void Update();
	void Draw(class CTransform* transform, class CMaterial* material, BlendMode blendMode, bool isHighlighted = false);
	void DrawShadow(class CTransform* transform, const DirectX::XMMATRIX& lightViewProj);

	void BindBoneSRV(D3D12_GPU_DESCRIPTOR_HANDLE handle);

	//???_?A?C???f?b?N?X????Z?b?g
	void SetVertex(const MeshVertex* vertices, size_t vertexCount,
		const uint32_t* indices, size_t indexCount);

	// Bounds
	DirectX::XMFLOAT3 GetLocalAABBMin() const { return m_localAABBMin; }
	DirectX::XMFLOAT3 GetLocalAABBMax() const { return m_localAABBMax; }

private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	D3D12_GPU_DESCRIPTOR_HANDLE m_BoneSrvGpuHandle;

	// Bounds
	DirectX::XMFLOAT3 m_localAABBMin{ -0.5f, -0.5f, -0.5f };
	DirectX::XMFLOAT3 m_localAABBMax{  0.5f,  0.5f,  0.5f };

	// 頂点・インデックス
	std::vector<MeshVertex> m_Vertices;
	std::vector<uint32_t>	m_Indices;

	void CalculateBounds();

private:
	CObject* m_Owner = nullptr;




	//----- Getter,Setter -----
public:
	//???_?f?[?^??��
	void RegisterOwner(CObject* _Owner);

	void SetBoneSRV(D3D12_GPU_DESCRIPTOR_HANDLE handle)
	{
		m_BoneSrvGpuHandle = handle;
	}
};

