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
class CTriangle
{
public:
	//コンストラクタ・デストラクタ
	CTriangle();

	//初期化処理
	void Initialize(ID3D12Device* _Device);//シェーダーコンパイルに関数名を変えてもいいかも 他の場所に移す

	void Draw(ID3D12GraphicsCommandList* _CommandList);

private:
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	//定数バッファ
	ComPtr<ID3D12Resource> m_constantBuffer;
	//定数バッファの保持
	ConstantBufferData* m_cbData = nullptr;

	// 位置（あとで当たり判定用にも使える）
	DirectX::XMFLOAT3 m_position = { 0,0,0 };
};

