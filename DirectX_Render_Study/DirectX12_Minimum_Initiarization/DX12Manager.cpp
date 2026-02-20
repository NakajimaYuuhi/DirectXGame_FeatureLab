//===== インクルード =====
#include "DX12Manager.h"

#include <d3dcompiler.h>
#include <iostream>

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
    swapChainDesc.Width = m_Width;
    swapChainDesc.Height = m_Height;
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


    return true;
}

//終了処理
void CDX12Manager::Finalize()
{
    m_commandQueue.Reset();
    m_device.Reset();
    m_factory.Reset();
}

void CDX12Manager::BeginDraw()
{
	// GPUが前のフレームの処理を終えるのを待つ
    if (m_fence->GetCompletedValue() < m_fenceValue - 1)
    {
        m_fence->SetEventOnCompletion(m_fenceValue - 1, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }


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

    // 5. クリア
    FLOAT clearColor[] = { 0.1f, 0.2f, 0.4f, 1.0f };

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
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
    m_swapChain->Present(1, 0);

	// 5. フェンスをキューに挿入
    const UINT64 fenceToWaitFor = m_fenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToWaitFor);
    m_fenceValue++;
}


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