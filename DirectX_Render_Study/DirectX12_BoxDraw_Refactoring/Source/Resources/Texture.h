#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include "DirectXTex.h"

#include <wrl.h>

using Microsoft::WRL::ComPtr;

class CTexture
{
public:
    CTexture()
        : m_cpuHandle{ 0 }
        , m_gpuHandle{ 0 }
    {
    }
    ~CTexture();

    CTexture(const CTexture&) = delete;
    CTexture& operator=(const CTexture&) = delete;

    bool LoadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const wchar_t* filePath);

    void CreateSRV(ID3D12Device* device);

    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() 
    { 
        return m_gpuHandle; 
    }

private:
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> uploadHeap;

    DirectX::TexMetadata metadata{};
    DirectX::ScratchImage scratch;

    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle{ 0 };
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle{ 0 };
};
