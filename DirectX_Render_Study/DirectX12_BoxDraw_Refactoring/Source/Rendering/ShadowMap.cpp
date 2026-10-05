#include "ShadowMap.h"
#include "DX12Manager.h"
#include <stdexcept>

ShadowMap::ShadowMap(ID3D12Device* device, UINT width, UINT height)
    : m_width(width)
    , m_height(height)
    , m_currentState(D3D12_RESOURCE_STATE_DEPTH_WRITE)
{
    m_viewport = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f };
    m_scissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    // 1. Create Texture Resource (R32_TYPELESS for Depth-Stencil & Shader Resource View)
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    resDesc.Width = width;
    resDesc.Height = height;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    resDesc.SampleDesc.Count = 1;
    resDesc.SampleDesc.Quality = 0;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    resDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;
    clearValue.DepthStencil.Stencil = 0;

    HRESULT hr = device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        m_currentState,
        &clearValue,
        IID_PPV_ARGS(&m_resource)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create ShadowMap resource.");
    }

    // 2. Create DSV Heap and View
    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    hr = device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create ShadowMap DSV Heap.");
    }

    m_dsvHandle = m_dsvHeap->GetCPUDescriptorHandleForHeapStart();

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
    device->CreateDepthStencilView(m_resource.Get(), &dsvDesc, m_dsvHandle);

    // 3. Allocate SRV Descriptor from main SRV Heap
    DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_srvCpuHandle, &m_srvGpuHandle);
    m_srvAllocated = true;

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Texture2D.MipLevels = 1;
    device->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srvCpuHandle);
}

ShadowMap::~ShadowMap()
{
    if (m_srvAllocated)
    {
        DX12Manager::GetInstance().GetSRVAllocator()->Free(m_srvCpuHandle, m_srvGpuHandle);
        m_srvAllocated = false;
    }
}

void ShadowMap::TransitionToDepthWrite(ID3D12GraphicsCommandList* cmdList)
{
    if (m_currentState == D3D12_RESOURCE_STATE_DEPTH_WRITE) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_resource.Get();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);
    m_currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;
}

void ShadowMap::TransitionToPixelShaderResource(ID3D12GraphicsCommandList* cmdList)
{
    if (m_currentState == D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE) return;

    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_resource.Get();
    barrier.Transition.StateBefore = m_currentState;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    cmdList->ResourceBarrier(1, &barrier);
    m_currentState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
}

void ShadowMap::Clear(ID3D12GraphicsCommandList* cmdList)
{
    cmdList->ClearDepthStencilView(m_dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
}
