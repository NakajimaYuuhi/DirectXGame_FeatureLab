#pragma once

#include <d3d12.h>
#include <memory>
#include <vector>
#include "RenderTexture.h"

// =========================================================================
// GBuffer
// Manages Multi-Render-Target (MRT) textures for Deferred Rendering:
// - RT0: Albedo (RGB) + Material ID / Specular (A)   [R8G8B8A8_UNORM]
// - RT1: World Normal (RGB) + Roughness (A)          [R16G16B16A16_FLOAT]
// - RT2: Metallic (R) + AO (G) + Emissive (B)        [R8G8B8A8_UNORM]
// =========================================================================
class GBuffer
{
public:
    enum TargetIndex
    {
        ALBEDO = 0,
        NORMAL = 1,
        MATERIAL = 2,
        COUNT = 3
    };

    GBuffer() = default;
    ~GBuffer() = default;

    GBuffer(const GBuffer&) = delete;
    GBuffer& operator=(const GBuffer&) = delete;

    void Init(ID3D12Device* pDevice, UINT width, UINT height);

    void TransitionToRenderTargets(ID3D12GraphicsCommandList* cmdList);
    void TransitionToShaderResources(ID3D12GraphicsCommandList* cmdList);

    void Clear(ID3D12GraphicsCommandList* cmdList);

    // Get RTV CPU handles for OMSetRenderTargets
    void GetRTVHandles(D3D12_CPU_DESCRIPTOR_HANDLE outHandles[COUNT]) const;

    RenderTexture* GetTexture(TargetIndex index) const { return m_targets[index].get(); }

    D3D12_GPU_DESCRIPTOR_HANDLE GetSRV(TargetIndex index) const
    {
        return m_targets[index] ? m_targets[index]->GetSRV() : D3D12_GPU_DESCRIPTOR_HANDLE{};
    }

    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }

private:
    UINT m_width = 0;
    UINT m_height = 0;

    std::unique_ptr<RenderTexture> m_targets[COUNT];
};
