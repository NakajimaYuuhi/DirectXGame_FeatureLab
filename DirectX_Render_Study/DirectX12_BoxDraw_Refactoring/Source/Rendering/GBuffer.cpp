#include "GBuffer.h"

void GBuffer::Init(ID3D12Device* pDevice, UINT width, UINT height)
{
    m_width = width;
    m_height = height;

    // Clear colors
    const float clearAlbedo[4]   = { 0.0f, 0.0f, 0.0f, 1.0f };
    const float clearNormal[4]   = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float clearMaterial[4] = { 0.0f, 1.0f, 1.0f, 0.0f }; // Metallic 0, Roughness 1, AO 1, Emissive 0

    // 0: Albedo (R8G8B8A8_UNORM)
    m_targets[ALBEDO] = std::make_unique<RenderTexture>(
        pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, clearAlbedo, false
    );

    // 1: World Normal (R16G16B16A16_FLOAT for high precision normal & depth)
    m_targets[NORMAL] = std::make_unique<RenderTexture>(
        pDevice, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT, clearNormal, false
    );

    // 2: Material Parameters (R8G8B8A8_UNORM)
    m_targets[MATERIAL] = std::make_unique<RenderTexture>(
        pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, clearMaterial, false
    );
}

void GBuffer::TransitionToRenderTargets(ID3D12GraphicsCommandList* cmdList)
{
    for (int i = 0; i < COUNT; ++i)
    {
        if (m_targets[i])
        {
            m_targets[i]->Transition(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
    }
}

void GBuffer::TransitionToShaderResources(ID3D12GraphicsCommandList* cmdList)
{
    for (int i = 0; i < COUNT; ++i)
    {
        if (m_targets[i])
        {
            m_targets[i]->Transition(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        }
    }
}

void GBuffer::Clear(ID3D12GraphicsCommandList* cmdList)
{
    for (int i = 0; i < COUNT; ++i)
    {
        if (m_targets[i])
        {
            m_targets[i]->Clear(cmdList);
        }
    }
}

void GBuffer::GetRTVHandles(D3D12_CPU_DESCRIPTOR_HANDLE outHandles[COUNT]) const
{
    for (int i = 0; i < COUNT; ++i)
    {
        outHandles[i] = m_targets[i] ? m_targets[i]->GetRTV() : D3D12_CPU_DESCRIPTOR_HANDLE{};
    }
}
