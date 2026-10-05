#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class ShadowMap
{
public:
    ShadowMap(ID3D12Device* device, UINT width = 2048, UINT height = 2048);
    ~ShadowMap();

    void TransitionToDepthWrite(ID3D12GraphicsCommandList* cmdList);
    void TransitionToPixelShaderResource(ID3D12GraphicsCommandList* cmdList);
    void Clear(ID3D12GraphicsCommandList* cmdList);

    D3D12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_dsvHandle; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srvGpuHandle; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetSRVCpu() const { return m_srvCpuHandle; }

    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    D3D12_VIEWPORT GetViewport() const { return m_viewport; }
    D3D12_RECT GetScissorRect() const { return m_scissorRect; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle{};

    D3D12_CPU_DESCRIPTOR_HANDLE m_srvCpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvGpuHandle{};
    bool m_srvAllocated = false;

    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_DEPTH_WRITE;

    UINT m_width;
    UINT m_height;
    D3D12_VIEWPORT m_viewport{};
    D3D12_RECT m_scissorRect{};
};
