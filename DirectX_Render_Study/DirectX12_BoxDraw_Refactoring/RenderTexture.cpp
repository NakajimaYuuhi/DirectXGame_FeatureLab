#include "RenderTexture.h"
#include <stdexcept>
// #include "DX12Manager.h" // ヒープやデバイス取得用

RenderTexture::RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format)
    : m_currentState(D3D12_RESOURCE_STATE_COMMON)
{
    // 1. テクスチャリソース(ID3D12Resource)の生成
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
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET; // RTVとして使うため必須

    // 初期状態はレンダリングターゲットとしてクリアできる状態にしておくと便利
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = format;
    clearValue.Color[0] = 0.0f; // R
    clearValue.Color[1] = 0.0f; // G
    clearValue.Color[2] = 0.0f; // B
    clearValue.Color[3] = 1.0f; // A

    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        m_currentState, // 初期状態
        &clearValue,
        IID_PPV_ARGS(&m_pResource)
    );
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create RenderTexture resource.");
    }

    // 2. RTVの作成 (RTV専用のヒープの空きスロットに作成する想定)
    // m_rtvHandle = DX12Manager::GetInstance()->AllocateRTV();
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    // pDevice->CreateRenderTargetView(m_pResource, &rtvDesc, m_rtvHandle);

    // 3. SRVの作成 (メインのCBV/SRV/UAVヒープの空きスロットに作成する想定)
    // m_srvHandle = TextureManager::GetInstance()->AllocateSRV(); // 既存ヒープの利用
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    // pDevice->CreateShaderResourceView(m_pResource, &srvDesc, m_srvHandle);
}

RenderTexture::~RenderTexture()
{
    if (m_pResource) {
        m_pResource->Release();
        m_pResource = nullptr;
    }
    // TODO: ここで確保した RTV/SRV ヒープのスロットを解放する処理を呼ぶ
}

void RenderTexture::Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState)
{
    if (m_currentState == nextState) return; // 既に同じ状態なら何もしない

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