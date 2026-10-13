#pragma once
#include <d3d12.h>
#include <wrl/client.h>

class RenderTexture {
public:
    RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format, const float clearColor[4] = nullptr, bool allowUAV = false);
    ~RenderTexture();

    RenderTexture(const RenderTexture&) = delete;
    RenderTexture& operator=(const RenderTexture&) = delete;

    void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState);
    void Clear(ID3D12GraphicsCommandList* cmdList, const float clearColor[4] = nullptr);
    void UAVBarrier(ID3D12GraphicsCommandList* cmdList);

    ID3D12Resource* GetResource() const { return m_pResource.Get(); }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_rtvHandleCPU; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srvHandleGPU; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetUAVCPU() const { return m_uavHandleCPU; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetUAV() const { return m_uavHandleGPU; }
    bool HasUAV() const { return m_uavAllocated; }

    D3D12_RESOURCE_STATES GetCurrentState() const { return m_currentState; }
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    DXGI_FORMAT GetFormat() const { return m_format; }
    const float* GetClearColor() const { return m_clearColor; }

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

    UINT m_width = 0;
    UINT m_height = 0;
    DXGI_FORMAT m_format = DXGI_FORMAT_UNKNOWN;
    float m_clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    // RTV descriptor heap
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_pRtvHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandleCPU{};

    // SRV handles allocated from main SRV heap
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvHandleCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandleGPU{};
    bool m_srvAllocated = false;

    // UAV handles allocated from main SRV/UAV heap
    D3D12_CPU_DESCRIPTOR_HANDLE m_uavHandleCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_uavHandleGPU{};
    bool m_uavAllocated = false;
};