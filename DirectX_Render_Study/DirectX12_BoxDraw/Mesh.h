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

	enum ColliderName
	{
		Ray,
		Plane,
	};


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
	//位置、回転、スケール
    DirectX::XMFLOAT3 m_Position	= { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_Rotation	= { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 m_Scale		= { 1.0f, 1.0f, 1.0f };

	//Front
	DirectX::XMFLOAT3 m_Front = { 0.0f, 0.5f, 0.5f };

	//速度
	DirectX::XMFLOAT3 m_Velocity = { 0.0f, 0.0f, 0.0f };

	//速度の大きさ
	float m_Speed = 0.15f;

	float m_Resistance = 0.995f;

	UniquePtr<CCollider> m_pCollider = nullptr;


	//----- Getter,Setter -----
public:
	DirectX::XMFLOAT3 GetPos() { return m_Position; }
	void	SetPos(DirectX::XMFLOAT3 _Position) 
	{ 
		m_Position = _Position; }

	DirectX::XMFLOAT3	GetScale() { return m_Scale; }
	void	SetScale(DirectX::XMFLOAT3 _Scale) { m_Scale = _Scale; }

	DirectX::XMFLOAT3	GetRotation() { return m_Rotation; }
	void	SetRotation(DirectX::XMFLOAT3 _Rotation) { m_Rotation = _Rotation; }

	DirectX::XMFLOAT3 GetVelocity() { return m_Velocity; }
	void SetVelocity(DirectX::XMFLOAT3 _Velocity) { m_Velocity = _Velocity; }

	void Dash()
	{
		//Frontの方向に速度を設定

		m_Velocity.x = m_Front.x * m_Speed;
		m_Velocity.y = m_Front.y * m_Speed;
		m_Velocity.z = m_Front.z * m_Speed;
	}

	void Resistance()
	{
		m_Velocity.x *= m_Resistance;
		m_Velocity.y *= m_Resistance;
		m_Velocity.z *= m_Resistance;
	}

	void Fall()
	{
		m_Velocity.y -= 0.0005f;
	}

	//Posの更新
	void Move()
	{
		m_Position.x += m_Velocity.x;
		m_Position.y += m_Velocity.y;
		m_Position.z += m_Velocity.z;
	}

	void ResetVelocityY()
	{
		m_Velocity.y = 0.0f;
	}

	//補正

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





	//Colliderの作成関数
	void MakeCollider(ColliderName _ColliderName);
	CCollider* GetCollider();
};

