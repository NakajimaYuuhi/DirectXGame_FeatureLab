//===== インクルード =====
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "BasicSettings.h"

//仮置き
#include "Box.h"
#include "Mesh.h"



using Microsoft::WRL::ComPtr;

//===== クラス定義 =====
class CDX12Manager
{
public:

	// <インスタンス取得>
	static CDX12Manager& GetInstance();

	// <初期化、終了処理>
	bool Initialize(HWND hwnd);
	void Finalize();

	// <描画処理>
	void BeginDraw();
	void EndDraw();


	// < 別で切り出したい >

	//更新処理
	void Update();




	void CreateCommandObjects();
	void CreateFence();

	//仮のview,projのGetter
	DirectX::XMMATRIX GetView() { return m_view; }
	DirectX::XMMATRIX GetProj() { return m_proj; }

	ID3D12Device* GetDevice() { return m_device.Get(); }
	ID3D12GraphicsCommandList* GetCommandLIst() { return m_commandList.Get(); }



private:

	//DirectX 12関連のメンバ変数
	ComPtr<IDXGIFactory6>       m_factory;      //ファクトリー
	ComPtr<ID3D12Device>        m_device;       //デバイス
	ComPtr<ID3D12CommandQueue>  m_commandQueue; //コマンドキュー

	ComPtr<IDXGISwapChain4> m_swapChain;        //スワップチェーン

	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;                     //RTVヒープ
	ComPtr<ID3D12Resource> m_renderTargets[FRAME_BUFFER_COUNT]; //レンダーターゲット

	UINT m_frameIndex = 0;					//フレームインデックス
	UINT m_rtvDescriptorSize = 0;								//RTVディスクリプタサイズ

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;			//コマンドアロケーター
	ComPtr<ID3D12GraphicsCommandList> m_commandList;			//コマンドリスト

	ComPtr<ID3D12Fence> m_fence;	//フェンス
	UINT64 m_fenceValue = 0;		//フェンス値
	HANDLE m_fenceEvent = nullptr;	//フェンスイベント

	// 深度バッファ
	ComPtr<ID3D12Resource> m_depthBuffer;

	// DSVヒープ
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;




	//画面関連
	static const UINT m_FrameBufferCount;       //フレームバッファの数
	UINT m_Width = SCREEN_WIDTH;                //画面の幅
	UINT m_Height = SCREEN_HEIGHT;              //画面の高さ



	//一旦持っておく
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_proj;


	//シングルトン実装
private:
	CDX12Manager() = default;
	~CDX12Manager() = default;

	CDX12Manager(const CDX12Manager&) = delete;
	CDX12Manager& operator=(const CDX12Manager&) = delete;

private:

};

