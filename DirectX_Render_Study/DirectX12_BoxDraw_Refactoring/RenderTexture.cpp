#include "RenderTexture.h"
#include <stdexcept>
#include "DX12Manager.h" // GetSRVHeap などを呼ぶため

RenderTexture::RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format)
    : m_currentState(D3D12_RESOURCE_STATE_COMMON)
{
    // 1. テクスチャリソースの生成
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = width;
    resDesc.Height = height;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = format;
    resDesc.SampleDesc.Count = 1;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = 0.0f;
    clearValue.Color[1] = 0.0f;
    clearValue.Color[2] = 0.0f;
    clearValue.Color[3] = 1.0f;

    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        m_currentState, &clearValue, IID_PPV_ARGS(&m_pResource)
    );
    if (FAILED(hr)) throw std::runtime_error("Failed to create RenderTexture resource.");

    // 2. RTV用ヒープの生成と View の作成 (自前で持つ
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap));

    m_rtvHandleCPU = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    pDevice->CreateRenderTargetView(m_pResource, &rtvDesc, m_rtvHandleCPU);

    // 3. SRV用ハンドルの計算と View の作成 (固定インデックス 100)
    // ※DX12Manager に GetSRVHeap() がある想定で書いています。コンパイルエラーが出たらその関数名を教えてください！
    UINT srvIndex = 100;
    ID3D12DescriptorHeap* pMainSrvHeap = DX12Manager::GetInstance().GetSRVHeap();
    UINT srvIncrement = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    
    m_srvHandleCPU = pMainSrvHeap->GetCPUDescriptorHandleForHeapStart();
    m_srvHandleCPU.ptr += (srvIndex * srvIncrement);
    
    m_srvHandleGPU = pMainSrvHeap->GetGPUDescriptorHandleForHeapStart();
    m_srvHandleGPU.ptr += (srvIndex * srvIncrement);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    pDevice->CreateShaderResourceView(m_pResource, &srvDesc, m_srvHandleCPU);
}

RenderTexture::~RenderTexture()
{
    if (m_pRtvHeap) m_pRtvHeap->Release();
    if (m_pResource) m_pResource->Release();
}

void RenderTexture::Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState)
{
    if (m_currentState == nextState) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pResource;
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = nextState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);
    m_currentState = nextState;
}