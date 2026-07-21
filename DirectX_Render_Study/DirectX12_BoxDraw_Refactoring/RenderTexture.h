#pragma once
#include <d3d12.h>

class RenderTexture {
public:
    RenderTexture(ID3D12Device* pDevice, UINT width, UINT height, DXGI_FORMAT format);
    ~RenderTexture();

    void Transition(ID3D12GraphicsCommandList* cmdList, D3D12_RESOURCE_STATES nextState);

    ID3D12Resource* GetResource() const { return m_pResource; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_rtvHandleCPU; }
    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV() const { return m_srvHandleGPU; }

private:
    ID3D12Resource* m_pResource = nullptr;
    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

    // RTVは自身でヒープを持つ
    ID3D12DescriptorHeap* m_pRtvHeap = nullptr;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandleCPU{};

    // SRVはメインヒープの固定スロットを間借りする
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvHandleCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvHandleGPU{};
};