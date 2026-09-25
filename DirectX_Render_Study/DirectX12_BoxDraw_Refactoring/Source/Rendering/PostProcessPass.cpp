#include "PostProcessPass.h"
#include <d3dcompiler.h>
#include <stdexcept>

// D3DCompile??g?????????C?u?????????N
#pragma comment(lib, "d3dcompiler.lib")

// ?K?v??????? d3dx12.h ??C???N???[?h??????i?v???W?F?N?g??????z??j
#include "d3dx12.h" 
#include "DX12Manager.h" // GetMainSRVHeap ????g??????

PostProcessPass::PostProcessPass(RenderTexture* pSourceTex)
    : m_pSourceTex(pSourceTex)
{
}

void PostProcessPass::Init(ID3D12Device* pDevice) {
    // --------------------------------------------------------
    // 1. ???[?g?V?O?l?`????? (SRV x1, ?T???v?? x1)
    // --------------------------------------------------------
    CD3DX12_DESCRIPTOR_RANGE1 srvRange;
    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL);

    // ?e?N?X?`????T???v?????O???????X?^?e?B?b?N?T???v??
    CD3DX12_STATIC_SAMPLER_DESC sampler(
        0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
        D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP
    );
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSigDesc;
    // ???_?o?b?t?@(InputLayout)??g??????? ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT ??O?????OK
    rootSigDesc.Init_1_1(1, rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ID3DBlob* signature = nullptr;
    ID3DBlob* error = nullptr;
    D3DX12SerializeVersionedRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1_1, &signature, &error);
    pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_pRootSignature));
    if (signature) signature->Release();
    if (error) error->Release();

    // --------------------------------------------------------
    // 2. ?V?F?[?_?[??R???p?C??
    // --------------------------------------------------------
    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
#if defined(_DEBUG)
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    D3DCompileFromFile(L"Assets/Shader/PostProcess.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vsBlob, nullptr);
    D3DCompileFromFile(L"Assets/Shader/PostProcess.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &psBlob, nullptr);

    // --------------------------------------------------------
    // 3. PSO (?p?C?v???C???X?e?[?g) ???
    // --------------------------------------------------------
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = m_pRootSignature;
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vsBlob);
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(psBlob);
    
    // ???_???C?A?E?g?????? (SV_VertexID??g??????)
    psoDesc.InputLayout = { nullptr, 0 };
    
    // ?u?????h?X?e?[?g????X?^???C?U (?f?t?H???g)
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    // ?J?????O???
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    
    psoDesc.DepthStencilState.DepthEnable = FALSE; // ?|?X?g?v???Z?X???[?x?e?X?g?s?v
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    // ?o?b?N?o?b?t?@??t?H?[?}?b?g??????? (???? R8G8B8A8_UNORM)
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; 
    psoDesc.SampleDesc.Count = 1;

    pDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pPipelineState));

    if (vsBlob) vsBlob->Release();
    if (psBlob) psBlob->Release();
}

void PostProcessPass::Execute(const RenderContext& ctx) {
    if (!m_pSourceTex || !m_pPipelineState) return;

    // 1. ?I?t?X?N???[???e?N?X?`???? ??????p (SRV) ??J??
    m_pSourceTex->Transition(ctx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    // 2. ?`????o?b?N?o?b?t?@?????A?N???A????
    ctx.cmdList->OMSetRenderTargets(1, &ctx.backBufferRTV, FALSE, nullptr);
    const float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ctx.cmdList->ClearRenderTargetView(ctx.backBufferRTV, clearColor, 0, nullptr);

    // ?r???[?|?[?g???
    D3D12_VIEWPORT viewport = { 0.0f, 0.0f, (float)ctx.screenWidth, (float)ctx.screenHeight, 0.0f, 1.0f };
    D3D12_RECT scissor = { 0, 0, (LONG)ctx.screenWidth, (LONG)ctx.screenHeight };
    ctx.cmdList->RSSetViewports(1, &viewport);
    ctx.cmdList->RSSetScissorRects(1, &scissor);

    // 3. PSO ??e?N?X?`????o?C???h
    ctx.cmdList->SetGraphicsRootSignature(m_pRootSignature);
    ctx.cmdList->SetPipelineState(m_pPipelineState);
    
    // SRV?q?[?v??Z?b?g (RenderTexture??????????ˆè?C??SRV?q?[?v)
    // ?? DX12Manager?? GetSRVHeap() ??????O????
    ID3D12DescriptorHeap* ppHeaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    ctx.cmdList->SetDescriptorHeaps(1, ppHeaps);
    
    // ?I?t?X?N???[???e?N?X?`???? SRV?n???h??(GPU??) ??V?F?[?_?[?? register(t0) ??n??
    ctx.cmdList->SetGraphicsRootDescriptorTable(0, m_pSourceTex->GetSRV());

    // 4. ?O?p?`??1???`?? (SV_VertexID??g??????S?????)
    ctx.cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    ctx.cmdList->DrawInstanced(3, 1, 0, 0);
}
