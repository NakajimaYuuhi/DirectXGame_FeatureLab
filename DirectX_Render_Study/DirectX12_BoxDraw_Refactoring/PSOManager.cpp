#include "PSOManager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void PSOManager::Init(ID3D12Device* device)
{
    HRESULT hr;

    // ----- Shader Compilation -----
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    hr = D3DCompileFromFile(
        L"Triangle.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        0,
        0,
        &vertexShader,
        &errorBlob
    );

    hr = D3DCompileFromFile(
        L"Triangle.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &errorBlob
    );


    // ----- Root Signature -----
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    D3D12_ROOT_PARAMETER rootParams[3] = {};

    // --Root Constants (b0)
    rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    rootParams[0].Constants.ShaderRegister = 0; // b0
    rootParams[0].Constants.RegisterSpace = 0;
    rootParams[0].Constants.Num32BitValues = 20; // WVP (16) + uvOffset (2) + uvScale (2) = 20 floats
    rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // --- SRV (t0 & t1)
    D3D12_DESCRIPTOR_RANGE srvRanges[2] = {};
    srvRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRanges[0].NumDescriptors = 1;
    srvRanges[0].BaseShaderRegister = 0; // t0
    srvRanges[0].RegisterSpace = 0;
    srvRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    srvRanges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srvRanges[1].NumDescriptors = 1;
    srvRanges[1].BaseShaderRegister = 1; // t1
    srvRanges[1].RegisterSpace = 0;
    srvRanges[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // 1. Texture SRV (t0)
    rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[1].DescriptorTable.pDescriptorRanges = &srvRanges[0];
    rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // 2. Bone SRV (t1)
    rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
    rootParams[2].DescriptorTable.pDescriptorRanges = &srvRanges[1];
    rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // --Sampler (s0)
    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    sampler.ShaderRegister = 0; // s0
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    // --RootSignatureDesc
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = _countof(rootParams);
    rootSignatureDesc.pParameters = rootParams;
    rootSignatureDesc.NumStaticSamplers = 1;
    rootSignatureDesc.pStaticSamplers = &sampler;
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );

    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_meshRootSignature)
    );

    // ----- Pipeline State Object -----
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);
    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;

    D3D12_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;
    rasterDesc.ForcedSampleCount = 0;
    rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    psoDesc.RasterizerState = rasterDesc;

    psoDesc.BlendState = {};
    psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;

    psoDesc.DepthStencilState = depthDesc;
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;
    psoDesc.pRootSignature = m_meshRootSignature.Get();

    hr = device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_meshPipelineState)
    );

    // ----- Additive Pipeline State Object -----
    D3D12_GRAPHICS_PIPELINE_STATE_DESC additivePsoDesc = psoDesc;
    
    // 加算合成 ブレンドステート設定    
    D3D12_RENDER_TARGET_BLEND_DESC blendDesc = {};
    blendDesc.BlendEnable = TRUE;
    blendDesc.LogicOpEnable = FALSE;
    blendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;         // SRC
    blendDesc.DestBlend = D3D12_BLEND_ONE;              // DEST (1 = 加算)
    blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    additivePsoDesc.BlendState.RenderTarget[0] = blendDesc;
    
    // Zバッファへの書き込みを無効化（半透 E描画の基本 E E    additivePsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

    hr = device->CreateGraphicsPipelineState(
        &additivePsoDesc,
        IID_PPV_ARGS(&m_additivePipelineState)
    );

    // ----- Sprite Shader Compilation -----
    ComPtr<ID3DBlob> spriteVertexShader;
    ComPtr<ID3DBlob> spritePixelShader;

    hr = D3DCompileFromFile(
        L"Sprite.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", 0, 0, &spriteVertexShader, &errorBlob
    );
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }

    hr = D3DCompileFromFile(
        L"Sprite.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", 0, 0, &spritePixelShader, &errorBlob
    );
    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
    }

    // ----- Sprite Root Signature -----
    D3D12_ROOT_PARAMETER spriteRootParams[2] = {};

    // Root Constants (b0) -> WVP (16 floats) + color (4 floats) = 20 floats
    spriteRootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    spriteRootParams[0].Constants.ShaderRegister = 0; // b0
    spriteRootParams[0].Constants.RegisterSpace = 0;
    spriteRootParams[0].Constants.Num32BitValues = 20; 
    spriteRootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    // Descriptor Table (t0) -> SRV
    D3D12_DESCRIPTOR_RANGE spriteSrvRange = {};
    spriteSrvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    spriteSrvRange.NumDescriptors = 1;
    spriteSrvRange.BaseShaderRegister = 0; // t0
    spriteSrvRange.RegisterSpace = 0;
    spriteSrvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    spriteRootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    spriteRootParams[1].DescriptorTable.NumDescriptorRanges = 1;
    spriteRootParams[1].DescriptorTable.pDescriptorRanges = &spriteSrvRange;
    spriteRootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC spriteSampler = sampler; // Reuse sampler from mesh PSO

    D3D12_ROOT_SIGNATURE_DESC spriteRootSignatureDesc{};
    spriteRootSignatureDesc.NumParameters = _countof(spriteRootParams);
    spriteRootSignatureDesc.pParameters = spriteRootParams;
    spriteRootSignatureDesc.NumStaticSamplers = 1;
    spriteRootSignatureDesc.pStaticSamplers = &spriteSampler;
    spriteRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    hr = D3D12SerializeRootSignature(
        &spriteRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error
    );

    hr = device->CreateRootSignature(
        0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_spriteRootSignature)
    );

    // ----- Sprite Pipeline State Object -----
    D3D12_INPUT_ELEMENT_DESC spriteInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC spritePsoDesc = {};
    spritePsoDesc.InputLayout.pInputElementDescs = spriteInputLayout;
    spritePsoDesc.InputLayout.NumElements = _countof(spriteInputLayout);
    spritePsoDesc.VS = { spriteVertexShader->GetBufferPointer(), spriteVertexShader->GetBufferSize() };
    spritePsoDesc.PS = { spritePixelShader->GetBufferPointer(), spritePixelShader->GetBufferSize() };
    spritePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    spritePsoDesc.SampleDesc.Count = 1;
    spritePsoDesc.SampleMask = UINT_MAX;
    
    // Rasterizer (No culling for 2D typically, or just backface)
    D3D12_RASTERIZER_DESC spriteRasterDesc = rasterDesc;
    spriteRasterDesc.CullMode = D3D12_CULL_MODE_NONE; // Often want none for 2D
    spriteRasterDesc.DepthClipEnable = FALSE;
    spritePsoDesc.RasterizerState = spriteRasterDesc;

    // Blend state (Alpha blending)
    D3D12_BLEND_DESC spriteBlendDesc = {};
    spriteBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    spriteBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    spriteBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    spriteBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    spriteBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    spriteBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    spriteBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    spriteBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    spritePsoDesc.BlendState = spriteBlendDesc;

    // Depth Stencil (No depth write/test for UI)
    D3D12_DEPTH_STENCIL_DESC spriteDepthDesc = {};
    spriteDepthDesc.DepthEnable = FALSE;
    spriteDepthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    spriteDepthDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    spriteDepthDesc.StencilEnable = FALSE;
    spritePsoDesc.DepthStencilState = spriteDepthDesc;

    spritePsoDesc.DSVFormat = DXGI_FORMAT_UNKNOWN; // No depth buffer used for 2D UI usually
    spritePsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    spritePsoDesc.NumRenderTargets = 1;
    spritePsoDesc.pRootSignature = m_spriteRootSignature.Get();

    hr = device->CreateGraphicsPipelineState(
        &spritePsoDesc,
        IID_PPV_ARGS(&m_spritePipelineState)
    );
}
