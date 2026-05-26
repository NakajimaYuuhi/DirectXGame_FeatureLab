//===== インクルード =====
#include "DX12Manager.h"

#include <d3dcompiler.h>
#include <iostream>

#include "InputManager.h"

#include "Windows.h"
#include "imgui.h"

#include "imgui_impl_dx12.h"

//===== 定数・マクロ定義 =====
const UINT CDX12Manager::m_FrameBufferCount = FRAME_BUFFER_COUNT;   //フレームバッファの数

//===== メソッド定義 =====

//インスタンス取得
CDX12Manager& CDX12Manager::GetInstance()
{
	static CDX12Manager instance;
	return instance;
}

// <初期化、終了処理>
//初期化処理
bool CDX12Manager::Initialize(HWND hwnd)
{
	HRESULT hr;

#if defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
	}
#endif

	//DXGI Factory 作成
	hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
	if (FAILED(hr))
		return false;

	//アダプタ取得
	ComPtr<IDXGIAdapter1> adapter;

	for (UINT i = 0;
		m_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
		++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		break;
	}

	//デバイス作成
	hr = D3D12CreateDevice(
		adapter.Get(),
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_device)
	);

	if (FAILED(hr))
		return false;

	//コマンドキュー作成
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	hr = m_device->CreateCommandQueue(
		&queueDesc,
		IID_PPV_ARGS(&m_commandQueue)
	);

	if (FAILED(hr))
		return false;


	// <スワップチェーン作成>
	
	//スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = m_FrameBufferCount;
	//0でウィンドウに合わせてもらう
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	//スワップチェーン作成
	ComPtr<IDXGISwapChain1> swapChain1;

	hr = m_factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain1
	);

	if (FAILED(hr))
		return false;

	//IDXGISwapChain4 に変換
	swapChain1.As(&m_swapChain);


	// <RTV作成>
	//RTVヒープの設定、作成
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = m_FrameBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = m_device->CreateDescriptorHeap(
		&rtvHeapDesc,
		IID_PPV_ARGS(&m_rtvHeap)
	);

	if (FAILED(hr))
		return false;

	//ディスクリプタサイズ取得
	m_rtvDescriptorSize =
		m_device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV
		);

	//バックバッファ取得＆RTV作成
	//ヒープの先頭ハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_FrameBufferCount; ++i)
	{
		//バックバッファ取得
		hr = m_swapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&m_renderTargets[i])
		);

		if (FAILED(hr))
			return false;

		//RTV作成
		m_device->CreateRenderTargetView(
			m_renderTargets[i].Get(),
			nullptr,
			rtvHandle
		);

		// 次のディスクリプタへ移動
		rtvHandle.ptr += m_rtvDescriptorSize;
	}


	//コマンドオブジェクト作成
	CreateCommandObjects();


	//フェンス作成
	CreateFence();

	//ImGUi作成、初期化






	// ===== 深度バッファ作成 =====
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC depthResourceDesc = {};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = SCREEN_WIDTH;
	depthResourceDesc.Height = SCREEN_HEIGHT;
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthBuffer)
	);


	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.NodeMask = 0;

	m_device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_dsvHeap)
	);


	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;

	m_device->CreateDepthStencilView(
		m_depthBuffer.Get(),
		&dsvDesc,
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);



	//SRV
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 128;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	hr = m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));

	m_srvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//ハンドルテスト
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();




	//view,projの初期化
	//view
	m_view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(10, 10, -20, 1),
		DirectX::XMVectorSet(0, 0, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));
;
//Todo : Modelの描画テストをするときは、正面からに直す


	//横から見る用
	m_view = DirectX::XMMatrixLookAtLH(
		DirectX::XMVectorSet(40, 0, 0, 1),
		DirectX::XMVectorSet(0, 0, 0, 1),
		DirectX::XMVectorSet(0, 1, 0, 0));

	//proj
	m_proj = DirectX::XMMatrixPerspectiveFovLH(
		DirectX::XM_PIDIV4,
		(float)SCREEN_WIDTH / SCREEN_HEIGHT,
		0.1f,
		100.0f);
	
	//m_commandQueue->ExecuteCommandLists(m_commandList);

	return true;
}

//終了処理
void CDX12Manager::Finalize()
{
	m_commandQueue.Reset();
	m_device.Reset();
	m_factory.Reset();
}


//----- 更新処理 -----
void CDX12Manager::Update()
{
}

void CDX12Manager::ForceWait()
{
	// 今積んである命令（コピーなど）に目印をつける
	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	// その目印に到達するまで、CPUを完全に停止させて待つ
	m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
	WaitForSingleObject(m_fenceEvent, INFINITE);

	// 次のために値を更新しておく
	m_fenceValue++;
}


//----- 描画処理 -----
void CDX12Manager::BeginDraw()
{





	// GPUが前のフレームの処理を終えるのを待つ
	if (m_fence->GetCompletedValue() < m_fenceValue - 1)
	{
		m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}


	// ここで一旦、今のコマンドリストの状態を強制的にクリアにする
	//m_commandAllocator->Reset();
	//m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	//変数のリセット
	ResetIsOccluded();

	// 1. フレームインデックス更新
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	// 2. リセット
	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);


	// 3. PRESENT → RENDER_TARGET へ遷移
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);

	// 4. RTVハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	rtvHandle.ptr += m_frameIndex * m_rtvDescriptorSize;






	D3D12_VIEWPORT viewport{};
	viewport.Width = (float)m_Width;
	viewport.Height = (float)m_Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = m_Width;
	scissorRect.bottom = m_Height;

	m_commandList->RSSetViewports(1, &viewport);
	m_commandList->RSSetScissorRects(1, &scissorRect);

	// 5. DSVハンドル取得
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart();


	// 6. クリア
	//画面の色
	FLOAT clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

	//レンダーターゲットのセット
	m_commandList->OMSetRenderTargets(
		1,
		&rtvHandle,
		FALSE,
		&dsvHandle
	);

	//RtV,DSVのセット
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(
		dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH,
		1.0f,
		0,
		0,
		nullptr
	);




}

void CDX12Manager::EndDraw()
{
	// 1. RENDER_TARGET → PRESENT
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_renderTargets[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	m_commandList->ResourceBarrier(1, &barrier);

	// 2. Close
	m_commandList->Close();

	// 3. 実行
	ID3D12CommandList* commandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(1, commandLists);

	// 4. Present
	HRESULT hr = m_swapChain->Present(1, 0);
	//HRESULT hr = g_pSwapChain->Present(0, g_SwapChainTearingSupport ? DXGI_PRESENT_ALLOW_TEARING : 0); // Present without vsync
	m_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);

	// 5. フェンスをキューに挿入
	const UINT64 fenceToWaitFor = m_fenceValue;
	m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
	m_fenceValue++;
}


//初期化用の関数達
void CDX12Manager::CreateCommandObjects()
{

	//コマンドアロケーター作成
	m_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&m_commandAllocator)
	);

	//コマンドリスト作成
	m_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_commandAllocator.Get(),
		nullptr,
		IID_PPV_ARGS(&m_commandList)
	);

	//コマンドリストは作成直後は recording 状態なので、Close しておく
	m_commandList->Close();
}

void CDX12Manager::CreateFence()
{
	m_device->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)
	);

	m_fenceValue = 1;

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}


void CDX12Manager::ResizeRenderTarget(LPARAM lParam)
{
	CleanupRenderTarget();              //RenderTargetの破棄
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	m_swapChain->GetDesc1(&desc);
	HRESULT result = m_swapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), desc.Format, desc.Flags);   //バッファーのリサイズ
	IM_ASSERT(SUCCEEDED(result) && "Failed to resize swapchain.");

	//m_commandAllocator->Reset();
	//m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	CreateRenderTarget();               //RenderTargetの生成

	//// ★ 5. 使い終わったら閉じて実行する（または次のBeginDrawに備えて閉じる）
	//m_commandList->Close();
	//ID3D12CommandList* lists[] = { m_commandList.Get() };
	//m_commandQueue->ExecuteCommandLists(1, lists);

	//// リサイズ直後も一応待っておくと安全！
	//WaitForPendingOperations();
}
void CDX12Manager::ResizeViewPort(LPARAM lParam)
{
	D3D12_VIEWPORT viewport{};
	viewport.Width = LOWORD(lParam);
	viewport.Height = HIWORD(lParam);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = LOWORD(lParam);
	scissorRect.bottom = HIWORD(lParam);

	//m_commandList->RSSetViewports(1, &viewport);
	//m_commandList->RSSetScissorRects(1, &scissorRect);
}
void CDX12Manager::ResizeDepthBuffer(LPARAM lParam)
{

	// 古いリソース破棄
	m_depthBuffer.Reset();
	m_dsvHeap.Reset();

	// 1. ヒーププロパティ（初期化時と同じ）
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// 2. リサイズ後の DepthStencil のリソース作成
	D3D12_RESOURCE_DESC depthResourceDesc = {};
	depthResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthResourceDesc.Alignment = 0;
	depthResourceDesc.Width = LOWORD(lParam);
	depthResourceDesc.Height = HIWORD(lParam);
	depthResourceDesc.DepthOrArraySize = 1;
	depthResourceDesc.MipLevels = 1;
	depthResourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthResourceDesc.SampleDesc.Count = 1;
	depthResourceDesc.SampleDesc.Quality = 0;
	depthResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	// Create depth buffer texture
	HRESULT hr = m_device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&depthResourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&m_depthBuffer)
	);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create depth buffer!\n");
		return;
	}

	// 3. DSV ヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	hr = m_device->CreateDescriptorHeap(
		&heapDesc,
		IID_PPV_ARGS(&m_dsvHeap)
	);
	if (FAILED(hr))
	{
		OutputDebugStringA("Failed to create DSV heap!\n");
		return;
	}

	// 4. DSV の作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	m_device->CreateDepthStencilView(
		m_depthBuffer.Get(),
		&dsvDesc,
		m_dsvHeap->GetCPUDescriptorHandleForHeapStart()
	);
}
void CDX12Manager::CreateRenderTarget()
{
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
		m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_FrameBufferCount; ++i)
	{
		//バックバッファ取得
		HRESULT hr = m_swapChain->GetBuffer(
			i,
			IID_PPV_ARGS(&m_renderTargets[i])
		);

		if (FAILED(hr))
			return;



		//RTV作成
		m_device->CreateRenderTargetView(
			m_renderTargets[i].Get(),
			nullptr,
			rtvHandle
		);

		// 次のディスクリプタへ移動
		rtvHandle.ptr += m_rtvDescriptorSize;
	}
}
void CDX12Manager::CleanupRenderTarget()
{
	WaitForPendingOperations();

	for (UINT i = 0; i < FRAME_BUFFER_COUNT; i++)
		m_renderTargets[i].Reset(); 
}

void CDX12Manager::WaitForPendingOperations()
{

	m_commandQueue->Signal(m_fence.Get(), m_fenceValue);

	if (m_fence->GetCompletedValue() < m_fenceValue)
	{
		m_fence->SetEventOnCompletion(m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_fenceValue++;

}

D3D12_GPU_DESCRIPTOR_HANDLE CDX12Manager::GetGpuSrvHandle(int index)
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), index, m_srvDescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE CDX12Manager::GetHeadGpuSrvHandle()
{
	return m_srvHeap->GetGPUDescriptorHandleForHeapStart();
}

D3D12_CPU_DESCRIPTOR_HANDLE CDX12Manager::GetCpuSrvHandle(int index)
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), index, m_srvDescriptorSize);
}


//
bool CDX12Manager::IsOccluded(HWND hwnd)
{
	return ((m_SwapChainOccluded && m_swapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) || IsIconic(hwnd));
}

void CDX12Manager::ResetIsOccluded()
{
	m_SwapChainOccluded = false;
}


