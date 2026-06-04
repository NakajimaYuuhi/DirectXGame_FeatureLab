#pragma once

#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

class PSOManager
{
public:
    static PSOManager& GetInstance()
    {
        static PSOManager instance;
        return instance;
    }

    void Init(ID3D12Device* device);

    ID3D12RootSignature* GetMeshRootSignature() const { return m_meshRootSignature.Get(); }
    ID3D12PipelineState* GetMeshPSO() const { return m_meshPipelineState.Get(); }

private:
    PSOManager() = default;
    ~PSOManager() = default;
    PSOManager(const PSOManager&) = delete;
    PSOManager& operator=(const PSOManager&) = delete;

    ComPtr<ID3D12RootSignature> m_meshRootSignature;
    ComPtr<ID3D12PipelineState> m_meshPipelineState;
};
