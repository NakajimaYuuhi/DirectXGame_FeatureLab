#include "PSOBuilder.h"
#include "ShaderManager.h"
#include "PSOManager.h"
#include "DX12Manager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void PSOManager::Init(ID3D12Device* device)
{
    // ===== 変数宣言 =====

	// --エラーハンドリング用
    HRESULT hr;                 // DirectX関連

    // --シェーダ関連
    // メッシュ用
    auto vertexShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Triangle.hlsl", "VSMain", "vs_5_0");
    auto pixelShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Triangle.hlsl", "PSMain", "ps_5_0");
    auto spriteVertexShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Sprite.hlsl", "VSMain", "vs_5_0");
    auto spritePixelShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Sprite.hlsl", "PSMain", "ps_5_0");

    // =========================================================
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

    // ----- スプライト用 PSO -----
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
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_UNKNOWN) // DepthBuffer使わなぁE
                  .SetRasterizerState(rasterDesc)
                  .SetDepthStencilState(depthDesc)
                  .SetBlendState(blendDesc);

        psoBuilder.Build(device, &m_spritePipelineState);
    }
}

ID3D12PipelineState* PSOManager::GetPSO(CMaterial* material, ID3D12RootSignature* rootSig)
{
    if (!material) return nullptr;

    // キャッシュキーの作成 (シェーダーファイル + エントリ + ブレンドモード)
    const std::string& vsEntry = material->GetVsEntry();
    std::wstring key = material->GetShaderFile() + L"_" + 
                       std::wstring(vsEntry.begin(), vsEntry.end()) + L"_" + 
                       std::to_wstring(static_cast<int>(material->GetBlendMode()));

    // キャッシュヒット
    if (m_psoCache.find(key) != m_psoCache.end())
    {
        return m_psoCache[key].Get();
    }

    // なければ作る
    ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
    
    auto vs = ShaderManager::GetInstance().GetShader(material->GetShaderFile().c_str(), material->GetVsEntry().c_str(), "vs_5_0");
    auto ps = ShaderManager::GetInstance().GetShader(material->GetShaderFile().c_str(), material->GetPsEntry().c_str(), "ps_5_0");

    if (!vs || !ps)
    {
        OutputDebugStringA(("Failed to load shader for material: " + material->GetVsEntry() + " / " + material->GetPsEntry() + "\n").c_str());
        return nullptr;
    }

    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    PSOBuilder psoBuilder;
    psoBuilder.SetRootSignature(rootSig)
              .SetInputLayout(inputLayout, _countof(inputLayout))
              .SetShaders(vs->GetBytecode(), ps->GetBytecode())
              .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
              .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);

    // ブレンドモードに応じた設?E
    if (material->GetBlendMode() == BlendMode::Additive)
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
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Z書き込みなぁE
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        psoBuilder.SetBlendState(blendDesc).SetDepthStencilState(depthDesc);
    }

    ComPtr<ID3D12PipelineState> newPso;
    psoBuilder.Build(device, &newPso);
    
    m_psoCache[key] = newPso;
    return newPso.Get();
}
