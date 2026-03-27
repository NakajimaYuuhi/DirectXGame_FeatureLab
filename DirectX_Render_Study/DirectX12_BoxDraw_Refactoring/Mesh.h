//Mesh.h
//メッシュのクラス

//===== インクルード =====
#pragma once

//----- DirectX12関連 -----
#include <d3d12.h>
#include <DirectXMath.h>

//----- スマートポインタ用 -----
#include <wrl.h>
using Microsoft::WRL::ComPtr;

#include <vector>


//----- Texture -----
//綺麗じゃないけど、一旦ここに入れる
#include "Texture.h"

//===== 前方宣言 =====
class CObject;
class CTransform;	//毎フレーム使うから置いとく
class CMaterial;

//===== 構造体定義(別の場所に移す) =====
struct MeshConstantBufferData
{
	DirectX::XMMATRIX WVP;
};

//===== クラス定義 =====
class CMesh
{
public:
	//Initializeをどこかで呼ぶ必要有り
	CMesh(CMaterial* _Material);

	//DX12Managerから取得する(仮)
	void Init();
	void Update();
	void Draw();

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


	//Material
	CMaterial* m_Material;

private:
	//Transform置き場
	CTransform* m_Transform = nullptr;
	void RegisterTransform();
	
	//CObjectの参照
	CObject* m_Owner;




	//----- Getter,Setter -----
public:
	//頂点データの取得
	void RegisterOwner(CObject* _Owner);
};

