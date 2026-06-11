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
class DX12Manager
{
public:

	// <インスタンス取得>
	static DX12Manager& GetInstance();

	// <初期化、終了処理>
	bool Initialize(HWND hwnd);
	void Finalize();

	// <描画処理>
	void BeginDraw();
	void EndDraw();


	// < 別で切り出したい >

	//更新処理
	void Update();


	void ResizeRenderTarget(LPARAM lParam);
	void ResizeDepthBuffer(LPARAM lParam);

	void ResizeViewPort(LPARAM lParam);


	void CreateCommandObjects();
	void CreateFence();

	void CreateRenderTarget();
	void CleanupRenderTarget();
	void WaitForPendingOperations();

	void ForceWait();

	//仮のview,projのGetter
	DirectX::XMMATRIX GetView() { return m_view; }
	DirectX::XMMATRIX GetProj() { return m_proj; }

	ID3D12Device* GetDevice() { return m_device.Get(); }
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }

	ID3D12DescriptorHeap* GetSRVHeap()
	{
		return m_srvHeap.Get();
	}

	ID3D12CommandAllocator* GetCommandAllocator() {
		return  m_commandAllocator.Get();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetGpuSrvHandle(int index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetHeadGpuSrvHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetCpuSrvHandle(int index);
	UINT AllocsrvNextIndex()
	{
		return m_srvNextIndex++;
	}

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


	//SRVは全体で1つ
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_srvDescriptorSize;
	UINT m_srvNextIndex = 0;







	//画面関連
	static const UINT m_FrameBufferCount;       //フレームバッファの数
	UINT m_Width = SCREEN_WIDTH;                //画面の幅
	UINT m_Height = SCREEN_HEIGHT;              //画面の高さ



	//一旦持っておく
	DirectX::XMMATRIX m_view;
	DirectX::XMMATRIX m_proj;


	//----- 画面が隠れているかの判定 -----
public:		
	bool IsOccluded(HWND hwnd);
private:	
	void ResetIsOccluded();
	bool m_SwapChainOccluded = false;


	//シングルトン実装
private:
	DX12Manager() = default;
	~DX12Manager() = default;

	DX12Manager(const DX12Manager&) = delete;
	DX12Manager& operator=(const DX12Manager&) = delete;

private:

};

