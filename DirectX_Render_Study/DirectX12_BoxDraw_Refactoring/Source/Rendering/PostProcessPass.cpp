#include "PostProcessPass.h"
#include <d3dcompiler.h>
#include <stdexcept>
#include "d3dx12.h"
#include "DX12Manager.h"
#include "LightManager.h"
#include "BasicSettings.h"
#include "ViewportUI.h"
#include "RenderBlackboard.h"

#pragma comment(lib, "d3dcompiler.lib")

PostProcessPass::PostProcessPass(RenderTexture* pSourceTex)
    : m_pSourceTex(pSourceTex)
{
    m_inputResourceName = "SceneColor";
    m_outputResourceName = "FinalColor";
}

std::string PostProcessPass::GetName() const
{
    if (!m_customName.empty()) {
        return m_customName;
    }
    if (m_settings.useCustomSettings) {
        if (m_settings.outlineEnabled && !m_settings.bloomEnabled) {
            return "PostProcess (Outline)";
        }
        if (m_settings.bloomEnabled) {
            return "PostProcess (Bloom & Tonemap)";
        }
        return "PostProcess (Screen Effect)";
    }
    return "PostProcess Pass (Bloom)";
}

void PostProcessPass::Init(ID3D12Device* pDevice)
{
    // --------------------------------------------------------
    // 1. Root Signature
    // Param 0: 32-bit Constants (24 DWORD = 96 bytes, register b0)
    // Param 1: Descriptor Table (1 SRV, register t0) - Input / Main
    // Param 2: Descriptor Table (1 SRV, register t1) - Bloom blur
    // Sampler 0: Static Sampler (register s0) - Linear Clamp
    // --------------------------------------------------------
    CD3DX12_ROOT_PARAMETER1 rootParams[3];

    // Param 0: Root Constants (24 DWORD = 96 bytes, register b0)
    rootParams[0].InitAsConstants(24, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 1: Descriptor Table t0
    CD3DX12_DESCRIPTOR_RANGE1 rangeT0;
    rangeT0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[1].InitAsDescriptorTable(1, &rangeT0, D3D12_SHADER_VISIBILITY_PIXEL);

    // Param 2: Descriptor Table t1
    CD3DX12_DESCRIPTOR_RANGE1 rangeT1;
    rangeT1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_NONE);
    rootParams[2].InitAsDescriptorTable(1, &rangeT1, D3D12_SHADER_VISIBILITY_PIXEL);

    // Static Sampler (s0: Linear Clamp)
    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0, // shaderRegister
        D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    rootSigDesc.Init_1_1(3, rootParams, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3DX12SerializeVersionedRootSignature(
        &rootSigDesc,
        D3D_ROOT_SIGNATURE_VERSION_1_1,
        &signatureBlob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        throw std::runtime_error("Failed to serialize PostProcess RootSignature");
    }

    hr = pDevice->CreateRootSignature(
        0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&m_pRootSignature)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create PostProcess RootSignature");
    }

    // --------------------------------------------------------
    // 2. Shader Compilation
    // --------------------------------------------------------
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psPassThroughBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBrightBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlurBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psCompositeBlob;

    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    auto CompileShader = [&](const char* entryPoint, const char* target, Microsoft::WRL::ComPtr<ID3DBlob>& blob)
    {
        Microsoft::WRL::ComPtr<ID3DBlob> error;
        HRESULT h = D3DCompileFromFile(
            L"Assets/Shader/PostProcess.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
            entryPoint, target, compileFlags, 0, &blob, &error
        );
        if (FAILED(h))
        {
            if (error) OutputDebugStringA((char*)error->GetBufferPointer());
            throw std::runtime_error(std::string("Failed to compile PostProcess shader: ") + entryPoint);
        }
    };

    CompileShader("VSMain",        "vs_5_0", vsBlob);
    CompileShader("PSPassThrough", "ps_5_0", psPassThroughBlob);
    CompileShader("PSBrightPass",  "ps_5_0", psBrightBlob);
    CompileShader("PSBlurPass",    "ps_5_0", psBlurBlob);
    CompileShader("PSComposite",   "ps_5_0", psCompositeBlob);

    // --------------------------------------------------------
    // 3. Build PSOs
    // --------------------------------------------------------
    auto BuildPSO = [&](ID3DBlob* ps, Microsoft::WRL::ComPtr<ID3D12PipelineState>& outPSO)
    {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.pRootSignature = m_pRootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(ps);

        psoDesc.InputLayout = { nullptr, 0 };

        CD3DX12_BLEND_DESC blendDesc(D3D12_DEFAULT);
        psoDesc.BlendState = blendDesc;

        CD3DX12_RASTERIZER_DESC rastDesc(D3D12_DEFAULT);
        rastDesc.CullMode = D3D12_CULL_MODE_NONE;
        psoDesc.RasterizerState = rastDesc;

        CD3DX12_DEPTH_STENCIL_DESC depthDesc(D3D12_DEFAULT);
        depthDesc.DepthEnable = FALSE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        psoDesc.DepthStencilState = depthDesc;

        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;

        HRESULT h = pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outPSO));
        if (FAILED(h))
        {
            throw std::runtime_error("Failed to create PostProcess PSO");
        }
    };

    BuildPSO(psPassThroughBlob.Get(), m_pPassThroughPSO);
    BuildPSO(psBrightBlob.Get(),      m_pBrightPSO);
    BuildPSO(psBlurBlob.Get(),        m_pBlurPSO);
    BuildPSO(psCompositeBlob.Get(),   m_pCompositePSO);

    // --------------------------------------------------------
    // 4. Create Half-Resolution Bloom Textures
    // --------------------------------------------------------
    UINT bloomWidth  = SCREEN_WIDTH / 2;
    UINT bloomHeight = SCREEN_HEIGHT / 2;

    m_pBrightTex   = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_pBlurTexTemp = std::make_unique<RenderTexture>(pDevice, bloomWidth, bloomHeight, DXGI_FORMAT_R8G8B8A8_UNORM);

    // --------------------------------------------------------
    // 5. Create Full-Resolution Output Texture (Transparent clear ready)
    // --------------------------------------------------------
    const float finalClear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    m_pFinalTex = std::make_unique<RenderTexture>(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, finalClear);
}

void PostProcessPass::Execute(const RenderContext& ctx)
{
    RenderTexture* pSource = nullptr;
    if (ctx.blackboard && !m_inputResourceName.empty()) {
        pSource = ctx.blackboard->GetTexture(m_inputResourceName);
    }
    if (!pSource) {
        pSource = m_pSourceTex;
    }
    if (!pSource || !m_pCompositePSO || !m_pFinalTex) return;

    if (ctx.blackboard && !m_outputResourceName.empty()) {
        ctx.blackboard->RegisterTextureRef(m_outputResourceName, m_pFinalTex.get());
    }

    // 0. Bind SRV Descriptor Heap
    ID3D12DescriptorHeap* heaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, heaps);

    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    const LightManager& lightMgr = LightManager::GetInstance();

    bool bloomEnabled = m_settings.useCustomSettings ? m_settings.bloomEnabled : lightMgr.IsBloomEnabled();
    float threshold = m_settings.useCustomSettings ? m_settings.bloomThreshold : lightMgr.GetBloomThreshold();
    float intensity = m_settings.useCustomSettings ? m_settings.bloomIntensity : lightMgr.GetBloomIntensity();
    float spread    = m_settings.useCustomSettings ? m_settings.bloomSpread    : lightMgr.GetBloomSpread();

    int effectType = m_settings.useCustomSettings ? m_settings.effectType : lightMgr.GetPostProcessEffectType();
    float chromaticAberration = m_settings.useCustomSettings
        ? (m_settings.chromaticAberrationEnabled ? m_settings.chromaticAberrationIntensity : 0.0f)
        : (lightMgr.IsChromaticAberrationEnabled() ? lightMgr.GetChromaticAberrationIntensity() : 0.0f);

    float outlineIntensity = m_settings.useCustomSettings
        ? (m_settings.outlineEnabled ? m_settings.outlineIntensity : 0.0f)
        : (lightMgr.IsOutlineEnabled() ? lightMgr.GetOutlineIntensity() : 0.0f);
    float outlineThreshold = m_settings.useCustomSettings ? m_settings.outlineThreshold : lightMgr.GetOutlineThreshold();
    float outlineWidth     = m_settings.useCustomSettings ? m_settings.outlineWidth     : lightMgr.GetOutlineWidth();
    float toneMapType      = static_cast<float>(m_settings.useCustomSettings ? m_settings.toneMapType : lightMgr.GetToneMapType());
    float exposure         = m_settings.useCustomSettings ? m_settings.exposure : lightMgr.GetExposure();

    float outlineColorR = m_settings.useCustomSettings ? m_settings.outlineColor[0] : 0.0f;
    float outlineColorG = m_settings.useCustomSettings ? m_settings.outlineColor[1] : 0.0f;
    float outlineColorB = m_settings.useCustomSettings ? m_settings.outlineColor[2] : 0.0f;
    float preserveAlpha = (m_settings.useCustomSettings && m_settings.preserveAlpha) ? 1.0f : 0.0f;

    float fadeColorR = m_settings.useCustomSettings ? m_settings.fadeColor[0] : 0.0f;
    float fadeColorG = m_settings.useCustomSettings ? m_settings.fadeColor[1] : 0.0f;
    float fadeColorB = m_settings.useCustomSettings ? m_settings.fadeColor[2] : 0.0f;
    float fade       = m_settings.useCustomSettings ? m_settings.fade         : 0.0f;

    struct PostProcessConstants
    {
        float threshold;
        float knee;
        float intensity;
        float spread;
        float dirX;
        float dirY;
        float bloomEnabled;
        float effectType;
        float chromaticAberration;
        float outlineIntensity;
        float outlineThreshold;
        float outlineWidth;
        float screenW;
        float screenH;
        float toneMapType;
        float exposure;
        float outlineColorR;
        float outlineColorG;
        float outlineColorB;
        float preserveAlpha;
        float fadeColorR;
        float fadeColorG;
        float fadeColorB;
        float fade;
    };

    D3D12_VIEWPORT vpFull = { 0.0f, 0.0f, static_cast<float>(ctx.screenWidth), static_cast<float>(ctx.screenHeight), 0.0f, 1.0f };
    D3D12_RECT scFull = { 0, 0, static_cast<LONG>(ctx.screenWidth), static_cast<LONG>(ctx.screenHeight) };

    PostProcessConstants cb{};
    cb.threshold           = threshold;
    cb.knee                = 0.2f;
    cb.intensity           = intensity;
    cb.spread              = spread;
    cb.dirX                = 0.0f;
    cb.dirY                = 0.0f;
    cb.bloomEnabled        = bloomEnabled ? 1.0f : 0.0f;
    cb.effectType          = static_cast<float>(effectType);
    cb.chromaticAberration = chromaticAberration;
    cb.outlineIntensity    = outlineIntensity;
    cb.outlineThreshold    = outlineThreshold;
    cb.outlineWidth        = outlineWidth;
    cb.screenW             = static_cast<float>(ctx.screenWidth);
    cb.screenH             = static_cast<float>(ctx.screenHeight);
    cb.toneMapType         = toneMapType;
    cb.exposure            = exposure;
    cb.outlineColorR       = outlineColorR;
    cb.outlineColorG       = outlineColorG;
    cb.outlineColorB       = outlineColorB;
    cb.preserveAlpha       = preserveAlpha;
    cb.fadeColorR          = fadeColorR;
    cb.fadeColorG          = fadeColorG;
    cb.fadeColorB          = fadeColorB;
    cb.fade                = fade;

    if (bloomEnabled && m_pBrightTex && m_pBlurTexTemp)
    {
        UINT bloomW = m_pBrightTex->GetWidth();
        UINT bloomH = m_pBrightTex->GetHeight();

        D3D12_VIEWPORT vpHalf = { 0.0f, 0.0f, static_cast<float>(bloomW), static_cast<float>(bloomH), 0.0f, 1.0f };
        D3D12_RECT scHalf = { 0, 0, static_cast<LONG>(bloomW), static_cast<LONG>(bloomH) };

        // Step 1: Bright Pass (pSource -> m_pBrightTex)
        pSource->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE brightRtv = m_pBrightTex->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpHalf);
        ctx.cmdList->RSSetScissorRects(1, &scHalf);

        ctx.cmdList->SetPipelineState(m_pBrightPSO.Get());
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &cb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, pSource->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // Step 2: Horizontal Blur (m_pBrightTex -> m_pBlurTexTemp)
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE blurRtv = m_pBlurTexTemp->GetRTV();
        ctx.cmdList->OMSetRenderTargets(1, &blurRtv, FALSE, nullptr);
        ctx.cmdList->SetPipelineState(m_pBlurPSO.Get());

        cb.dirX = 1.0f / static_cast<float>(bloomW);
        cb.dirY = 0.0f;
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &cb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // Step 3: Vertical Blur (m_pBlurTexTemp -> m_pBrightTex)
        m_pBlurTexTemp->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        ctx.cmdList->OMSetRenderTargets(1, &brightRtv, FALSE, nullptr);

        cb.dirX = 0.0f;
        cb.dirY = 1.0f / static_cast<float>(bloomH);
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &cb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pBlurTexTemp->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);

        // Step 4: Composite (pSource + m_pBrightTex -> m_pFinalTex)
        m_pBrightTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE finalRtv = m_pFinalTex->GetRTV();
        if (preserveAlpha > 0.5f) {
            const float clearZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            ctx.cmdList->ClearRenderTargetView(finalRtv, clearZero, 0, nullptr);
        }
        ctx.cmdList->OMSetRenderTargets(1, &finalRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pCompositePSO.Get());
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &cb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, pSource->GetSRV());
        ctx.cmdList->SetGraphicsRootDescriptorTable(2, m_pBrightTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }
    else
    {
        // PassThrough + Outline / Filter / Tonemap / Fade (pSource -> m_pFinalTex)
        pSource->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);

        D3D12_CPU_DESCRIPTOR_HANDLE finalRtv = m_pFinalTex->GetRTV();
        if (preserveAlpha > 0.5f) {
            const float clearZero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            ctx.cmdList->ClearRenderTargetView(finalRtv, clearZero, 0, nullptr);
        }
        ctx.cmdList->OMSetRenderTargets(1, &finalRtv, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &cb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, pSource->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }

    // Transition m_pFinalTex to PIXEL_SHADER_RESOURCE for Viewport / subsequent passes
    m_pFinalTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    CViewportUI::GetInstance().SetTextureSRV(m_pFinalTex->GetSRV());

    // Output to BackBuffer if enabled and ViewportUI is hidden
    bool doBackBuffer = !m_settings.useCustomSettings || m_settings.outputToBackBuffer;
#ifdef _DEBUG
    if (CViewportUI::GetInstance().IsVisible())
    {
        doBackBuffer = false;
    }
#endif
    if (doBackBuffer)
    {
        PostProcessConstants copyCb{};
        copyCb.screenW     = static_cast<float>(ctx.screenWidth);
        copyCb.screenH     = static_cast<float>(ctx.screenHeight);
        copyCb.toneMapType = 0.0f;
        copyCb.exposure    = 1.0f;

        ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
        ctx.cmdList->RSSetViewports(1, &vpFull);
        ctx.cmdList->RSSetScissorRects(1, &scFull);

        ctx.cmdList->SetPipelineState(m_pPassThroughPSO.Get());
        ctx.cmdList->SetGraphicsRoot32BitConstants(0, 24, &copyCb, 0);
        ctx.cmdList->SetGraphicsRootDescriptorTable(1, m_pFinalTex->GetSRV());
        ctx.cmdList->DrawInstanced(3, 1, 0, 0);
    }
}
