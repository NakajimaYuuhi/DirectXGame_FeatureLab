//===== インクルード =====
#pragma once

//----- DirectX12関連 -----
#include <d3d12.h>

//----- スマートポインタ用 -----
#include <wrl.h>
using Microsoft::WRL::ComPtr;


//===== 前方宣言 =====


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
};

