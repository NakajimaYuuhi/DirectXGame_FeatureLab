#include "SampleComputePass.h"
#include "PSOBuilder.h"
#include "Shader.h"
#include "DX12Manager.h"
#include "BasicSettings.h"
#include "ViewportUI.h"

SampleComputePass::SampleComputePass()
{
}

void SampleComputePass::Init(ID3D12Device* pDevice)
{
    // 1. RootSignatureBuilder
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE);
    // Param 0: 32-bit Constants (b0) - float g_Time, float3 g_Padding (4 DWORDs)
    rootSigBuilder.AddConstants(4, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
    // Param 1: UAV Descriptor Table (u0) - RWTexture2D<float4>
    rootSigBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    if (!rootSigBuilder.Build(pDevice, m_pRootSignature.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build Compute RootSignature!\n");
        return;
    }

    // 2. Compile Compute Shader
    Shader csShader;
    if (!csShader.CompileFromFile(L"Assets/Shader/SampleCompute.hlsl", "CSMain", "cs_5_0"))
    {
        OutputDebugStringA("Failed to compile SampleCompute.hlsl!\n");
        return;
    }

    // 3. Compute PSO
    ComputePSOBuilder psoBuilder;
    psoBuilder.SetRootSignature(m_pRootSignature.Get());
    psoBuilder.SetComputeShader(csShader.GetBytecode());

    if (!psoBuilder.Build(pDevice, m_pPipelineState.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build Compute PSO!\n");
        return;
    }

    // 4. Output RenderTexture with UAV enabled
    m_pOutputTexture = std::make_unique<RenderTexture>(
        pDevice,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        nullptr,
        true // allowUAV
    );
}

void SampleComputePass::Execute(const RenderContext& ctx)
{
    if (!m_isEnabled || !m_pOutputTexture || !m_pPipelineState || !m_pRootSignature)
    {
        return;
    }

    m_totalTime += ctx.deltaTime * m_speed;

    // 1. Pipeline State & Root Signature
    ctx.cmdList->SetPipelineState(m_pPipelineState.Get());
    ctx.cmdList->SetComputeRootSignature(m_pRootSignature.Get());

    // 2. Set Descriptor Heaps
    ID3D12DescriptorHeap* ppHeaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // 3. Transition RenderTexture to UNORDERED_ACCESS
    m_pOutputTexture->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

    // 4. Set Compute Root Parameters
    struct ComputeConstants {
        float time;
        float pad[3];
    } constants = { m_totalTime, { 0.0f, 0.0f, 0.0f } };

    ctx.cmdList->SetComputeRoot32BitConstants(0, 4, &constants, 0);
    ctx.cmdList->SetComputeRootDescriptorTable(1, m_pOutputTexture->GetUAV());

    // 5. Dispatch
    UINT threadGroupX = (m_pOutputTexture->GetWidth() + 7) / 8;
    UINT threadGroupY = (m_pOutputTexture->GetHeight() + 7) / 8;
    ctx.cmdList->Dispatch(threadGroupX, threadGroupY, 1);

    // 6. UAV Barrier to ensure write completes
    m_pOutputTexture->UAVBarrier(ctx.cmdList);

    // 7. Transition back to PIXEL_SHADER_RESOURCE for reading
    m_pOutputTexture->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 8. If preview enabled, send to ViewportUI
    if (m_previewInViewport && CViewportUI::GetInstance().IsVisible())
    {
        CViewportUI::GetInstance().SetTextureSRV(m_pOutputTexture->GetSRV());
    }
}
