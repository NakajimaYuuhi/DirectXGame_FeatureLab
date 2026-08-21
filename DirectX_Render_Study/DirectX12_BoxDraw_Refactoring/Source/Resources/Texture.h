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
    {
    }
    ~CTexture();

    bool LoadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,const wchar_t* filePath);

    void  CreateSRV(ID3D12Device* device);


    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() 
    { 
        return m_gpuHandle; 
    }


    // SRV  GPU nhiMesh ?gpjÇ≈égÇ§ópÅj

private:
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> uploadHeap;


    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratch;

    D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;
};