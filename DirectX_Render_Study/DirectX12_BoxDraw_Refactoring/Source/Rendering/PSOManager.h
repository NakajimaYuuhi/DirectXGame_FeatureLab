#pragma once
#include "Material.h"
#include <string>
#include <unordered_map>

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

	// 蜍慕噪PSO逕滓・
	ID3D12PipelineState* GetPSO(CMaterial* material, ID3D12RootSignature* rootSig);

private:
	// PSO繧ｭ繝｣繝・す繝･: 繧ｭ繝ｼ縺ｯ "繧ｷ繧ｧ繝ｼ繝繝ｼ繝代せ_繝悶Ξ繝ｳ繝峨Δ繝ｼ繝・ 縺ｪ縺ｩ縺ｮ荳諢上・譁・ｭ怜・
	std::unordered_map<std::wstring, ComPtr<ID3D12PipelineState>> m_psoCache;


public:
    ID3D12RootSignature* GetMeshRootSignature() const { return m_meshRootSignature.Get(); }
    ID3D12PipelineState* GetMeshPSO() const { return m_meshPipelineState.Get(); }
    ID3D12PipelineState* GetAdditivePSO() const { return m_additivePipelineState.Get(); }

    ID3D12RootSignature* GetSpriteRootSignature() const { return m_spriteRootSignature.Get(); }
    ID3D12PipelineState* GetSpritePSO() const { return m_spritePipelineState.Get(); }

private:
    PSOManager() = default;
    ~PSOManager() = default;
    PSOManager(const PSOManager&) = delete;
    PSOManager& operator=(const PSOManager&) = delete;

    ComPtr<ID3D12RootSignature> m_meshRootSignature;
    ComPtr<ID3D12PipelineState> m_meshPipelineState;
    ComPtr<ID3D12PipelineState> m_additivePipelineState;

    ComPtr<ID3D12RootSignature> m_spriteRootSignature;
    ComPtr<ID3D12PipelineState> m_spritePipelineState;
};
