//===== インクルード =====
#pragma once

//----- DirectX12関連 -----
#include <d3d12.h>
#include <DirectXMath.h>

//----- スマートポインタ用 -----
#include <wrl.h>
using Microsoft::WRL::ComPtr;


//===== 前方宣言 =====
struct ConstantBufferData
{
	DirectX::XMMATRIX WVP;
};

//===== クラス定義 =====
class CBox
{
public:
	//コンストラクタ・デストラクタ
	CBox();

	//初期化処理
	void Initialize(ID3D12Device* _Device);//シェーダーコンパイルに関数名を変えてもいいかも 他の場所に移す

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
	ConstantBufferData* m_cbData = nullptr;

	//// 深度バッファ
	//ComPtr<ID3D12Resource> m_depthBuffer;

	//// DSVヒープ
	//ComPtr<ID3D12DescriptorHeap> m_dsvHeap;

	// 位置（あとで当たり判定用にも使える）
	DirectX::XMFLOAT3 m_Position = { 0.0f,0.0f,0.0f };
	//大きさ
	DirectX::XMFLOAT3 m_Scale = { 1.0f, 1.0f, 1.0f };

//----- Getter,Setter -----
public:
	DirectX::XMFLOAT3 GetPos()		{ return m_Position;	}
	void	SetPos(DirectX::XMFLOAT3 _Position) { m_Position = _Position; }

	DirectX::XMFLOAT3	GetScale()	{ return m_Scale;		}
	void	SetScale(DirectX::XMFLOAT3 _Scale)	{ m_Scale = _Scale; }
};

