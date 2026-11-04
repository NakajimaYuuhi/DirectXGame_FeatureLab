#include "GBufferPass.h"
#include "ObjectManager.h"
#include "DX12Manager.h"
#include "PSOManager.h"
#include "ViewportUI.h"
#include "RenderBlackboard.h"
#include "RenderLayer.h"
#include "BasicSettings.h"

GBufferPass::GBufferPass()
{
    m_outputResourceName = "GBuffer_Albedo";
}

void GBufferPass::Init(ID3D12Device* pDevice)
{
    if (!m_gBuffer)
    {
        m_gBuffer = std::make_unique<GBuffer>();
        m_gBuffer->Init(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT);
    }
}

void GBufferPass::SetFilterPreset(ObjectFilterPreset preset)
{
    m_filterPreset = preset;
    switch (preset)
    {
    case ObjectFilterPreset::All:
        m_tagMask = 0xFFFFFFFFu;
        break;
    case ObjectFilterPreset::GroundAndBackground:
        m_tagMask = (1u << ObjectTag::BACKGROUND) |
                    (1u << ObjectTag::FIELD) |
                    (1u << ObjectTag::TRIANGLE) |
                    (1u << ObjectTag::BILLBOARD);
        break;
    case ObjectFilterPreset::Characters:
        m_tagMask = (1u << ObjectTag::PLAYER) |
                    (1u << ObjectTag::ENEMY) |
                    (1u << ObjectTag::PLAYER_BULLET) |
                    (1u << ObjectTag::ENEMY_BULLET);
        break;
    case ObjectFilterPreset::UIOnly:
        m_tagMask = (1u << ObjectTag::UI) |
                    (1u << ObjectTag::TEXT) |
                    (1u << ObjectTag::MANAGER);
        break;
    case ObjectFilterPreset::Custom:
    default:
        break;
    }
}

void GBufferPass::Execute(const RenderContext& ctx)
{
    if (!m_gBuffer)
    {
        Init(DX12Manager::GetInstance().GetDevice());
        if (!m_gBuffer) return;
    }

    // Register textures to blackboard if available
    if (ctx.blackboard)
    {
        ctx.blackboard->RegisterTextureRef("GBuffer_Albedo",   m_gBuffer->GetTexture(GBuffer::ALBEDO));
        ctx.blackboard->RegisterTextureRef("GBuffer_Normal",   m_gBuffer->GetTexture(GBuffer::NORMAL));
        ctx.blackboard->RegisterTextureRef("GBuffer_Material", m_gBuffer->GetTexture(GBuffer::MATERIAL));
        ctx.blackboard->RegisterTextureRef("GBuffer_WorldPos", m_gBuffer->GetTexture(GBuffer::POSITION));
    }

    // 0. Bind main SRV descriptor heap
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    // 1. Transition GBuffer textures to RENDER_TARGET state
    m_gBuffer->TransitionToRenderTargets(ctx.cmdList);

    // 2. Clear GBuffer render targets
    m_gBuffer->Clear(ctx.cmdList);

    // 3. Set render targets (MRT 4) and main depth stencil
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[GBuffer::COUNT];
    m_gBuffer->GetRTVHandles(rtvHandles);
    ctx.cmdList->OMSetRenderTargets(GBuffer::COUNT, rtvHandles, FALSE, &ctx.mainDSV);

    // 4. Clear depth stencil
    if (m_clearDepthEnabled)
    {
        ctx.cmdList->ClearDepthStencilView(ctx.mainDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }

    // 5. Viewport and Scissor
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // 6. Draw scene geometry with GBuffer PSO override
    ID3D12PipelineState* gbufferPSO = PSOManager::GetInstance().GetGBufferPSO();
    if (gbufferPSO)
    {
        PSOManager::GetInstance().SetActiveOverridePSO(gbufferPSO);
        ObjectManager::GetInstance().DrawFiltered(m_tagMask, m_layerFilter);
        PSOManager::GetInstance().SetActiveOverridePSO(nullptr);
    }

    // 7. Transition GBuffer textures to PIXEL_SHADER_RESOURCE state for subsequent lighting pass
    m_gBuffer->TransitionToShaderResources(ctx.cmdList);

    // 8. Update Viewport preview
    CViewportUI::GetInstance().SetTextureSRV(m_gBuffer->GetSRV(GBuffer::ALBEDO));
}

std::string GBufferPass::GetName() const
{
    if (!m_customName.empty())
    {
        return m_customName;
    }
    return "GBuffer Pass (Deferred MRT)";
}
