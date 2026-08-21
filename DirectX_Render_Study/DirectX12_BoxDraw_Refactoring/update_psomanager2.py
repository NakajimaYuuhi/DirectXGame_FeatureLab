import re

with open('PSOManager.cpp', 'r', encoding='shift_jis', errors='ignore') as f:
    text = f.read()

# インクルード追加
if '#include "PSOBuilder.h"' not in text:
    text = '#include "PSOBuilder.h"\n' + text

# =========================================================
# 2. ルートシグネチャ作成以降をまるごと置き換えるためのパターン
# =========================================================

# 置換開始の目印
pattern = r'// =========================================================\n\s*//\s*2\..*?hr = device->CreateGraphicsPipelineState\(\n\s*&spritePsoDesc,\n\s*IID_PPV_ARGS\(&m_spritePipelineState\)\n\s*\);'

replacement = r'''// =========================================================
    //  2. ルートシグネチャ作成 
    // =========================================================

    // ----- メッシュ用ルートシグネチャ -----
    {
        RootSignatureBuilder rsBuilder;
        rsBuilder.AddConstants(20, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // WVP + uvOffset + uvScale
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // Texture
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // Bone

        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ShaderRegister = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(sampler);

        rsBuilder.Build(device, &m_meshRootSignature);
    }

    // ----- スプライト用ルートシグネチャ -----
    {
        RootSignatureBuilder rsBuilder;
        rsBuilder.AddConstants(20, 0, 0, D3D12_SHADER_VISIBILITY_ALL); // WVP + color
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ShaderRegister = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(sampler);

        rsBuilder.Build(device, &m_spriteRootSignature);
    }


    // =========================================================
    //  3. PSOの構築
    // =========================================================

    // ----- メッシュ用 InputLayout -----
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // ----- 通常メッシュPSO -----
    {
        PSOBuilder psoBuilder;
        psoBuilder.SetRootSignature(m_meshRootSignature.Get())
                  .SetInputLayout(inputLayout, _countof(inputLayout))
                  .SetShaders(vertexShader->GetBytecode(), pixelShader->GetBytecode())
                  .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);
        
        psoBuilder.Build(device, &m_meshPipelineState);
    }

    // ----- 加算合成メッシュPSO -----
    {
        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = TRUE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Z書き込みなし
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

        PSOBuilder psoBuilder;
        psoBuilder.SetRootSignature(m_meshRootSignature.Get())
                  .SetInputLayout(inputLayout, _countof(inputLayout))
                  .SetShaders(vertexShader->GetBytecode(), pixelShader->GetBytecode())
                  .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT)
                  .SetBlendState(blendDesc)
                  .SetDepthStencilState(depthDesc);
        
        psoBuilder.Build(device, &m_additivePipelineState);
    }

    // ----- スプライト用 InputLayout -----
    D3D12_INPUT_ELEMENT_DESC spriteInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ----- スプライトPSO -----
    {
        D3D12_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDesc.CullMode = D3D12_CULL_MODE_NONE; // カリングなし
        rasterDesc.DepthClipEnable = FALSE;

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = FALSE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

        D3D12_BLEND_DESC blendDesc = {};
        blendDesc.RenderTarget[0].BlendEnable = TRUE;
        blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        PSOBuilder psoBuilder;
        psoBuilder.SetRootSignature(m_spriteRootSignature.Get())
                  .SetInputLayout(spriteInputLayout, _countof(spriteInputLayout))
                  .SetShaders(spriteVertexShader->GetBytecode(), spritePixelShader->GetBytecode())
                  .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_UNKNOWN) // DepthBuffer使わない
                  .SetRasterizerState(rasterDesc)
                  .SetDepthStencilState(depthDesc)
                  .SetBlendState(blendDesc);

        psoBuilder.Build(device, &m_spritePipelineState);
    }'''

text = re.sub(pattern, replacement, text, flags=re.DOTALL)

with open('PSOManager.cpp', 'w', encoding='shift_jis') as f:
    f.write(text)
