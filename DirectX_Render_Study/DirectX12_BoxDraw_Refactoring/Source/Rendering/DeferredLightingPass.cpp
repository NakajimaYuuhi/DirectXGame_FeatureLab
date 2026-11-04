#include "DeferredLightingPass.h"
#include "ShaderManager.h"
#include "PSOBuilder.h"
#include "LightManager.h"
#include "RenderBlackboard.h"
#include "ViewportUI.h"
#include "DX12Manager.h"

DeferredLightingPass::DeferredLightingPass(RenderTexture* pDestTex)
    : m_pDestTex(pDestTex)
{
    m_outputResourceName = "SceneColor";
}

void DeferredLightingPass::Init(ID3D12Device* pDevice)
{
    BuildRootSignatureAndPSO(pDevice);
}

void DeferredLightingPass::BuildRootSignatureAndPSO(ID3D12Device* pDevice)
{
    // 1. Root Signature
    {
        RootSignatureBuilder rsBuilder;

        // 0: LightBuffer (register b0)
        rsBuilder.AddConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        // Individual Descriptor Tables for GBuffer textures (t0..t3)
        // 1: Albedo (t0)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        // 2: Normal (t1)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        // 3: Material (t2)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        // 4: WorldPos (t3)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        // Individual Descriptor Tables for Shadow Maps (t4..t6)
        // 5: ShadowMap0 (t4)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 4, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        // 6: ShadowMap1 (t5)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 5, 0, D3D12_SHADER_VISIBILITY_PIXEL);
        // 7: ShadowMap2 (t6)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 6, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        // Sampler 0: Linear Clamp for GBuffer
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler.ShaderRegister = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(sampler);

        // Sampler 1: Linear Border White for ShadowMap
        D3D12_STATIC_SAMPLER_DESC shadowSampler{};
        shadowSampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        shadowSampler.ShaderRegister = 1;
        shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(shadowSampler);

        rsBuilder.Build(pDevice, &m_rootSignature);
    }

    // 2. Pipeline State
    {
        auto vs = ShaderManager::GetInstance().GetShader(L"Assets/Shader/DeferredLighting.hlsl", "VSMain", "vs_5_0");
        auto ps = ShaderManager::GetInstance().GetShader(L"Assets/Shader/DeferredLighting.hlsl", "PSMain", "ps_5_0");

        if (vs && ps && m_rootSignature)
        {
            D3D12_RASTERIZER_DESC rasterDesc = {};
            rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
            rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
            rasterDesc.DepthClipEnable = FALSE;

            D3D12_DEPTH_STENCIL_DESC depthDesc = {};
            depthDesc.DepthEnable = FALSE;
            depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
            depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

            DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

            PSOBuilder psoBuilder;
            psoBuilder.SetRootSignature(m_rootSignature.Get())
                      .SetShaders(vs->GetBytecode(), ps->GetBytecode())
                      .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                      .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_UNKNOWN)
                      .SetRasterizerState(rasterDesc)
                      .SetDepthStencilState(depthDesc)
                      .SetDefaultBlendState();

            psoBuilder.Build(pDevice, &m_pipelineState);
        }
    }
}

bool DeferredLightingPass::ReloadShaders(ID3D12Device* pDevice, std::string* outError)
{
    BuildRootSignatureAndPSO(pDevice);
    return (m_pipelineState != nullptr);
}

void DeferredLightingPass::Execute(const RenderContext& ctx)
{
    if (!m_pipelineState || !m_rootSignature)
    {
        BuildRootSignatureAndPSO(DX12Manager::GetInstance().GetDevice());
        if (!m_pipelineState) return;
    }

    // Resolve output render target
    RenderTexture* pDest = nullptr;
    if (ctx.blackboard && !m_outputResourceName.empty())
    {
        pDest = ctx.blackboard->GetTexture(m_outputResourceName);
    }
    if (!pDest)
    {
        pDest = m_pDestTex;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = ctx.backBufferRTV;
    if (pDest)
    {
        pDest->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
        rtvHandle = pDest->GetRTV();
    }

    // Set render target
    ctx.cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    if (m_clearColor)
    {
        const float defaultClear[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
        const float* cc = pDest ? pDest->GetClearColor() : defaultClear;
        ctx.cmdList->ClearRenderTargetView(rtvHandle, cc, 0, nullptr);
    }

    // Viewport & Scissor
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissorRect);

    // Bind descriptor heap
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    // Set Pipeline & Root Signature
    ctx.cmdList->SetPipelineState(m_pipelineState.Get());
    ctx.cmdList->SetGraphicsRootSignature(m_rootSignature.Get());

    // 0: LightBuffer (b0)
    D3D12_GPU_VIRTUAL_ADDRESS lightCbGpu = LightManager::GetInstance().GetConstantBufferGPUAddress();
    if (lightCbGpu != 0)
    {
        ctx.cmdList->SetGraphicsRootConstantBufferView(0, lightCbGpu);
    }

    // 1..4: GBuffer textures (t0..t3)
    if (ctx.blackboard)
    {
        RenderTexture* texAlbedo   = ctx.blackboard->GetTexture("GBuffer_Albedo");
        RenderTexture* texNormal   = ctx.blackboard->GetTexture("GBuffer_Normal");
        RenderTexture* texMaterial = ctx.blackboard->GetTexture("GBuffer_Material");
        RenderTexture* texWorldPos = ctx.blackboard->GetTexture("GBuffer_WorldPos");

        if (texAlbedo)   ctx.cmdList->SetGraphicsRootDescriptorTable(1, texAlbedo->GetSRV());
        if (texNormal)   ctx.cmdList->SetGraphicsRootDescriptorTable(2, texNormal->GetSRV());
        if (texMaterial) ctx.cmdList->SetGraphicsRootDescriptorTable(3, texMaterial->GetSRV());
        if (texWorldPos) ctx.cmdList->SetGraphicsRootDescriptorTable(4, texWorldPos->GetSRV());
    }

    // 5..7: ShadowMaps (t4..t6)
    for (int c = 0; c < 3; ++c)
    {
        ShadowMap* sm = LightManager::GetInstance().GetShadowMap(c);
        if (sm && sm->GetSRV().ptr != 0)
        {
            ctx.cmdList->SetGraphicsRootDescriptorTable(5 + c, sm->GetSRV());
        }
    }

    // Draw full-screen triangle
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(3, 1, 0, 0);

    // Update Viewport preview
    if (pDest)
    {
        CViewportUI::GetInstance().SetTextureSRV(pDest->GetSRV());
    }
}

std::string DeferredLightingPass::GetName() const
{
    if (!m_customName.empty())
    {
        return m_customName;
    }
    return "Deferred Lighting Pass";
}
