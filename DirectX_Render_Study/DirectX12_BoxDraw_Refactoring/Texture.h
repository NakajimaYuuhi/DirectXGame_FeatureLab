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

    bool LoadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList,const wchar_t* filePath, int srvIndex);

    void  CreateSRV(ID3D12Device* device);


    D3D12_GPU_DESCRIPTOR_HANDLE GetGpuHandle() 
    { 
        return m_gpuHandle; 
    }


    int GetSrvIndex() {
        return m_srvIndex;
    }
    // SRV の GPU ハンドル（Mesh で使う用）

private:
    ComPtr<ID3D12Resource> texture;
    ComPtr<ID3D12Resource> uploadHeap;


    DirectX::TexMetadata metadata;
    DirectX::ScratchImage scratch;

    D3D12_GPU_DESCRIPTOR_HANDLE m_gpuHandle;

    //インデックス
    int m_srvIndex;
};