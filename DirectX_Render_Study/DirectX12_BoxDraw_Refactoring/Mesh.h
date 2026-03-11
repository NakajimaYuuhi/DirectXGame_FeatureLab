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

#include "Component.h"


template <typename T>
using UniquePtr = std::unique_ptr<T>;



//===== 前方宣言 =====
class CTransform;	//毎フレーム使うから置いとく

//===== 構造体定義(別の場所に移す) =====
struct MeshConstantBufferData
{
	DirectX::XMMATRIX WVP;
};

//===== クラス定義 =====
class CMesh : public CComponent
{
public:
	//Initializeをどこかで呼ぶ必要有り
	CMesh()
		:CComponent("Mesh")
	{

	}

	//DX12Managerからしゅとくする
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
	//Transform置き場
	CTransform* m_Transform;
	void RegisterTransform();

	//----- Getter,Setter -----
public:
	//頂点データの取得
};

