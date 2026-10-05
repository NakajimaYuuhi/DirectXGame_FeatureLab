#include "PostProcessPass.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include "d3dx12.h"
#include "DX12Manager.h"
#include "LightManager.h"
#include "BasicSettings.h"

#pragma comment(lib, "d3dcompiler.lib")

PostProcessPass::PostProcessPass(RenderTexture* pSourceTex)
    : m_pSourceTex(pSourceTex)
{
}

void PostProcessPass::Init(ID3D12Device* pDevice)
{
    // --------------------------------------------------------
    // 1. 繝ｫ繝ｼ繝医す繧ｰ繝阪メ繝｣菴懈・
    // Param 0: 32-bit Constants (8 DWORD = 32 bytes, register b0)
    // Param 1: Descriptor Table (1 SRV, register t0) - Input / Main
    // Param 2: Descriptor Table (1 SRV, register t1) - Bloom blur
    // Sampler 0: Static Sampler (register s0) - Linear Clamp
    // --------------------------------------------------------
    CD3DX12_ROOT_PARAMETER1 rootParams[3];
    
    // Param 0: Root Constants
    rootParams[0].InitAsConstants(8, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 1: Descriptor Table t0
    CD3DX12_DESCRIPTOR_RANGE1 rangeT0;
    rangeT0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[1].InitAsDescriptorTable(1, &rangeT0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 2: Descriptor Table t1
    CD3DX12_DESCRIPTOR_RANGE1 rangeT1;
    rangeT1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[2].InitAsDescriptorTable(1, &rangeT1, D3D12_SHADER_VISIBILITY_PIXEL);

    // Sampler: Linear Clamp
    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0,
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(3, rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> sigBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &sigBlob, &errBlob);
    if (FAILED(hr))
    {
        if (errBlob)
        {
            OutputDebugStringA((char*)errBlob->GetBufferPointer());
        }
        throw std::runtime_error("Failed to serialize PostProcess root signature.");
    }

    hr = pDevice->CreateRootSignature(0, sigBlob->GetBufferPointer(), sigBlob->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create PostProcess root signature.");
    }

    // --------------------------------------------------------
    // 2. 繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｮ繧ｳ繝ｳ繝代う繝ｫ
    // --------------------------------------------------------
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psPassThroughBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBrightBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlurBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psCompositeBlob;

    auto CompileShader = [&](const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        HRESULT res = D3DCompileFromFile(
            L"Assets/Shader/PostProcess.hlsl", nullptr, nullptr,
            entryPoint, target, compileFlags, 0, &blob, &error
        );
        if (FAILED(res))
        {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            throw std::runtime_error(std::string("Failed to compile shader: ") + entryPoint);
        }
    };

    CompileShader("VSMain",        "vs_5_0", vsBlob);
    CompileShader("PSPassThrough", "ps_5_0", psPassThroughBlob);
    CompileShader("PSBrightPass",  "ps_5_0", psBrightBlob);
    CompileShader("PSBlurPass",    "ps_5_0", psBlurBlob);
    CompileShader("PSComposite",   "ps_5_0", psCompositeBlob);

    // --------------------------------------------------------
    // 3. 蜷・ｨｮ PSO (Pipeline State Object) 縺ｮ讒狗ｯ・
    // --------------------------------------------------------
    auto BuildPSO = [&](ID3DBlob* psBytecode, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pso)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_pRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBytecode);
        psoDesc.InputLayout = { nullptr, 0 };
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        HRESULT buildHr = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pso));
        if (FAILED(buildHr))
        {
            throw std::runtime_error("Failed to create PostProcess PSO.");
        }
    };

    BuildPSO(psPassThroughBlob.Get(), m_pPassThroughPSO);
    BuildPSO(psBrightBlob.Get(),      m_pBrightPSO);
    BuildPSO(psBlurBlob.Get(),        m_pBlurPSO);
    BuildPSO(psCompositeBlob.Get(),   m_pCompositePSO);

    // --------------------------------------------------------
    // 4. 繝悶Ν繝ｼ繝菴懈･ｭ逕ｨ 1/2 隗｣蜒丞ｺｦ繝・け繧ｹ繝√Ε縺ｮ菴懈・
    // --------------------------------------------------------
    UINT bloomWidth  = SCREEN_WIDTH / 2;
    UINT bloomHeight = SCREEN_HEIGHT / 2;

    m_pBrightTex   = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pBlurTexTemp = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
}

void PostProcessPass::Execute(const RenderContext& ctx)
{
    if (!m_pSourceTex || !m_pCompositePSO) return;

    // 0. SRV繝・ぅ繧ｹ繧ｯ繝ｪ繝励ち繝偵・繝励ｒ繝舌う繝ｳ繝・
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const LightManager& lightMgr = LightManager::GetInstance();
    bool bloomEnabled = lightMgr.IsBloomEnabled();

    struct PostProcessConstants
    {
        float threshold;
        float knee;
        float intensity;
        float spread;
        float dirX;
        float dirY;
        float bloomEnabled;
        float padding;
    };

    if (bloomEnabled && m_pBrightTex && m_pBlurTexTemp)
    {
        float threshold = lightMgr.GetBloomThreshold();
        float intensity = lightMgr.GetBloomIntensity();
        float spread    = lightMgr.GetBloomSpread();

        UINT bloomW = m_pBrightTex->GetWidth();
        UINT bloomH = m_pBrightTex->GetHeight();

        D3D12_VIEWPORT vpHalf = { 0.0f, 0.0f, static_cast<float>(bloomW), static_cast<float>(bloomH), 0.0f, 1.0f };
        D3D12_RECT scHalf = { 0, 0, static_cast<LONG>(bloomW), static_cast<LONG>(bloomH) };

        // --------------------------------------------------------
        // Step 1: 鬮倩ｼ晏ｺｦ謚ｽ蜃ｺ (Bright Pass: SceneTex -> BrightTex)
        // --------------------------------------------------------
        m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE brightRtv = m_pBrightTex->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpHalf);
        ctx.cmdList->RSSetScissorRects(1, &scHalf);

        ctx.cmdList->SetPipelineState(m_pBrightPSO.Get());

        PostProcessConstants cb{};
        cb.threshold    = threshold;
        cb.knee         = 0.2f;
        cb.intensity    = intensity;
        cb.spread       = spread;
        cb.dirX         = 0.0f;
        cb.dirY         = 0.0f;
        cb.bloomEnabled = 1.0f;
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 2: 豌ｴ蟷ｳ繧ｬ繧ｦ繧ｹ繝悶Λ繝ｼ (Horizontal Blur: BrightTex -> BlurTexTemp)
        // --------------------------------------------------------
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE blurRtv = m_pBlurTexTemp->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &blurRtv, FALSE, nullptr);

        ctx.cmdList->SetPipelineState(m_pBlurPSO.Get());

        cb.dirX = 1.0f / static_cast<float>(bloomW);
        cb.dirY = 0.0f;
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 3: 蝙ら峩繧ｬ繧ｦ繧ｹ繝悶Λ繝ｼ (Vertical Blur: BlurTexTemp -> BrightTex)
        // --------------------------------------------------------
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);

        cb.dirX = 0.0f;
        cb.dirY = 1.0f / static_cast<float>(bloomH);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBlurTexTemp->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // --------------------------------------------------------
        // Step 4: 譛邨ょ粋謌・(Composite: SceneTex + BrightTex -> BackBuffer)
        // --------------------------------------------------------
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
        D3D12_VIEWPORT vpFull = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
        D3D12_RECT scFull = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pCompositePSO.Get());

        cb.bloomEnabled = 1.0f;
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 8, &cb, 0);

        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->SetGraphicsRootDescriptorTable(2, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }
    else
    {
        // --------------------------------------------------------
        // 繝悶Ν繝ｼ繝辟｡蜉ｹ譎・ 繝代せ繧ｹ繝ｫ繝ｼ (SceneTex -> BackBuffer)
        // --------------------------------------------------------
        m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

        ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
        D3D12_VIEWPORT vpFull = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
        D3D12_RECT scFull = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pSourceTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }
}