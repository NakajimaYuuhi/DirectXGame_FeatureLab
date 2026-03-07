//Mesh.h
//メッシュのクラス

//===== インクルード =====
#pragma once

//----- DirectX12関連 -----
#include <d3d12.h>
#include <DirectXMath.h>
#include <memory>

//----- スマートポインタ用 -----
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <vector>

//#include "Collider.h"


template <typename T>
using UniquePtr = std::unique_ptr<T>;



//===== 前方宣言 =====
class CCollider;

//===== 構造体定義(別の場所に移す) =====
struct MeshConstantBufferData
{
	DirectX::XMMATRIX WVP;
};

//===== クラス定義 =====
class CMesh
{
public:
	void Initialize(ID3D12Device* _Device);
	void Update();
	void Draw(ID3D12GraphicsCommandList* _CommandList);

private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	//定数バッファ
	ComPtr<ID3D12Resource> m_constantBuffer;
	//定数バッファの保持
	MeshConstantBufferData* m_cbData = nullptr;

private:
	// <Transformに移動したい>

	//位置、回転、スケール
    DirectX::XMFLOAT3 m_Position	= { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_Rotation	= { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_Scale		= { 1.0f, 1.0f, 1.0f };


	//----- Getter,Setter -----
public:

	// <Transform>

	DirectX::XMFLOAT3 GetPos() { return m_Position; }
	void	SetPos(DirectX::XMFLOAT3 _Position) { m_Position = _Position; }

	DirectX::XMFLOAT3	GetScale() { return m_Scale; }
	void	SetScale(DirectX::XMFLOAT3 _Scale) { m_Scale = _Scale; }

	DirectX::XMFLOAT3	GetRotation() { return m_Rotation; }
	void	SetRotation(DirectX::XMFLOAT3 _Rotation) { m_Rotation = _Rotation; }

	//Frontの取得
	DirectX::XMFLOAT3 GetFront() 
	{ 
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
		
		//回転行列からFrontを取り出す
		DirectX::XMFLOAT3 front;
		front = { rotMat.r[2].m128_f32[0], rotMat.r[2].m128_f32[1], rotMat.r[2].m128_f32[2] };
		
		return front;
	}

	//Upの取得
	DirectX::XMFLOAT3 GetUp()
	{
		//回転行列の作成
		DirectX::XMMATRIX rotMat = DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);

		//回転行列からFrontを取り出す
		DirectX::XMFLOAT3 up;
		up = { rotMat.r[1].m128_f32[0], rotMat.r[1].m128_f32[1], rotMat.r[1].m128_f32[2] };

		return up;
	}

	DirectX::XMMATRIX GetWorld();
};

