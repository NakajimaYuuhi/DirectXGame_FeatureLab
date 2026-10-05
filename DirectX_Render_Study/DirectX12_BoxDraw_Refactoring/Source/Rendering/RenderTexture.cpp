#include "RenderTexture.h"
#include <stdexcept>
#include <cstring>
#include "DX12Manager.h"

RenderTexture::RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format, const float clearColor[4])
    : m_width(width)
    , m_height(height)
    , m_format(format)
    , m_currentState(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE)
{
    if (clearColor)
    {
        std::memcpy(m_clearColor, clearColor, sizeof(float) * 4);
    }
    else
    {
        m_clearColor[0] = 0.0f;
        m_clearColor[1] = 0.0f;
        m_clearColor[2] = 0.0f;
        m_clearColor[3] = 1.0f;
    }

    // 1. Create Texture Resource
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
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_CLEAR_VALUE d3dClearValue = {};
    d3dClearValue.Format = format;
    std::memcpy(d3dClearValue.Color, m_clearColor, sizeof(float) * 4);

    HRESULT hr = pDevice->CreateCommittedResource(
        &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
        m_currentState, &d3dClearValue, IID_PPV_ARGS(&m_pResource)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create RenderTexture resource.");
    }

    // 2. Create RTV Descriptor Heap & View
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = 1;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = pDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_pRtvHeap));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create RenderTexture RTV Heap.");
    }

    m_rtvHandleCPU = m_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = format;
    rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    pDevice->CreateRenderTargetView(m_pResource.Get(), &rtvDesc, m_rtvHandleCPU);

    // 3. Allocate SRV from main descriptor heap
    DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_srvHandleCPU, &m_srvHandleGPU);
    m_srvAllocated = true;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    pDevice->CreateShaderResourceView(m_pResource.Get(), &srvDesc, m_srvHandleCPU);
}

RenderTexture::~RenderTexture()
{
    if (m_srvAllocated)
    {
        DX12Manager::GetInstance().GetSRVAllocator()->Free(m_srvHandleCPU, m_srvHandleGPU);
        m_srvAllocated = false;
    }
}

void RenderTexture::Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState)
{
    if (m_currentState == nextState) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = m_pResource.Get();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = nextState;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);
    m_currentState = nextState;
}

void RenderTexture::Clear(ID3D12GraphicsCommandList* cmdList, const float clearColor[4])
{
    const float* colorToUse = clearColor ? clearColor : m_clearColor;
    cmdList->ClearRenderTargetView(m_rtvHandleCPU, colorToUse, 0, nullptr);
}