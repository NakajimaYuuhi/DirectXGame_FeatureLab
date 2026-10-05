#include "PSOBuilder.h"
#include "ShaderManager.h"
#include "PSOManager.h"
#include "DX12Manager.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

void PSOManager::Init(ID3D12Device* device)
{
    // ===== ????? =====

    // --エラーハンドリング用

    // --?V?F?[?_??A
    // ???b?V???p
    auto vertexShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Triangle.hlsl", "VSMain", "vs_5_0");
    auto pixelShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Triangle.hlsl", "PSMain", "ps_5_0");
    auto spriteVertexShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Sprite.hlsl", "VSMain", "vs_5_0");
    auto spritePixelShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/Sprite.hlsl", "PSMain", "ps_5_0");

    // =========================================================
    //  2. ???[?g?V?O?l?`????
    // =========================================================

    // ----- メッシュ用ルートシグネチャ -----
    {
        RootSignatureBuilder rsBuilder;
        rsBuilder.AddConstants(36, 0, 0, D3D12_SHADER_VISIBILITY_ALL); // 0: WVP(16) + World(16) + UV(4) = 36 DWORD, register(b0)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_SHADER_VISIBILITY_PIXEL); // 1: Texture register(t0)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // 2: Bone register(t1)
        rsBuilder.AddConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // 3: LightBuffer register(b1) (Root CBV)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_SHADER_VISIBILITY_PIXEL); // 4: ShadowMap register(t2)

        // Static Sampler 0: s0 (Material Linear Wrap)
        D3D12_STATIC_SAMPLER_DESC sampler{};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.ShaderRegister = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(sampler);

        // Static Sampler 1: s1 (Shadow Map Linear Border White)
        D3D12_STATIC_SAMPLER_DESC shadowSampler{};
        shadowSampler.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
        shadowSampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        shadowSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
        shadowSampler.ShaderRegister = 1;
        shadowSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
        rsBuilder.AddStaticSampler(shadowSampler);

        rsBuilder.Build(device, &m_meshRootSignature);
    }

    // ----- シャドウ用ルートシグネチャ -----
    {
        RootSignatureBuilder rsBuilder;
        rsBuilder.AddConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX); // 0: LightWVP (16 DWORD), register(b0)
        rsBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // 1: Bone register(t1)

        rsBuilder.Build(device, &m_shadowRootSignature);
    }

    // ----- ?X?v???C?g?p???[?g?V?O?l?`?? -----
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
    //  3. PSO??\?z
    // =========================================================

    // ----- ???b?V???p InputLayout -----
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "BLENDWEIGHT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    // ----- ????b?V??PSO -----
    {
        PSOBuilder psoBuilder;
        psoBuilder.SetRootSignature(m_meshRootSignature.Get())
                  .SetInputLayout(inputLayout, _countof(inputLayout))
                  .SetShaders(vertexShader->GetBytecode(), pixelShader->GetBytecode())
                  .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_D32_FLOAT);
        
        psoBuilder.Build(device, &m_meshPipelineState);
    }

    // ----- ???Z???????b?V??PSO -----
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
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Z??????????
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

    // ----- シャドウ用 PSO (深度のみ描画) -----
    auto shadowVertexShader = ShaderManager::GetInstance().GetShader(L"Assets/Shader/ShadowMap.hlsl", "VSMain", "vs_5_0");
    if (shadowVertexShader)
    {
        D3D12_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
        rasterDesc.FrontCounterClockwise = FALSE;
        rasterDesc.DepthBias = 500;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.SlopeScaledDepthBias = 1.5f;
        rasterDesc.DepthClipEnable = TRUE;

        D3D12_DEPTH_STENCIL_DESC depthDesc = {};
        depthDesc.DepthEnable = TRUE;
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

        PSOBuilder psoBuilder;
        psoBuilder.SetRootSignature(m_shadowRootSignature.Get())
                  .SetInputLayout(inputLayout, _countof(inputLayout))
                  .SetShaders(shadowVertexShader->GetBytecode(), { nullptr, 0 })
                  .SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE)
                  .SetRenderTargetFormats(0, nullptr, DXGI_FORMAT_D32_FLOAT)
                  .SetRasterizerState(rasterDesc)
                  .SetDepthStencilState(depthDesc);

        psoBuilder.Build(device, &m_shadowPipelineState);
    }

    // ----- ?X?v???C?g?p InputLayout -----
    D3D12_INPUT_ELEMENT_DESC spriteInputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    // ----- ?X?v???C?g?p PSO -----
    {
        D3D12_RASTERIZER_DESC rasterDesc = {};
        rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
        rasterDesc.CullMode = D3D12_CULL_MODE_NONE; // ?J?????O???
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
                  .SetRenderTargetFormats(1, &rtvFormat, DXGI_FORMAT_UNKNOWN) // DepthBuffer?g????E
                  .SetRasterizerState(rasterDesc)
                  .SetDepthStencilState(depthDesc)
                  .SetBlendState(blendDesc);

        psoBuilder.Build(device, &m_spritePipelineState);
    }

    // ----- ダミーボーンバッファ（ボーンを持たないメッシュの安全対策） -----
    {
        D3D12_HEAP_PROPERTIES heapProp = {};
        heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Width = sizeof(DirectX::XMMATRIX);
        resDesc.Height = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels = 1;
        resDesc.Format = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        device->CreateCommittedResource(
            &heapProp, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&m_dummyBoneBuffer)
        );

        DirectX::XMMATRIX identityMat = DirectX::XMMatrixIdentity();
        void* mapped = nullptr;
        m_dummyBoneBuffer->Map(0, nullptr, &mapped);
        memcpy(mapped, &identityMat, sizeof(DirectX::XMMATRIX));
        m_dummyBoneBuffer->Unmap(0, nullptr);

        DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_dummyBoneSrvCpuHandle, &m_dummyBoneSrvGpuHandle);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = DXGI_FORMAT_UNKNOWN;
        srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.FirstElement = 0;
        srvDesc.Buffer.NumElements = 1;
        srvDesc.Buffer.StructureByteStride = sizeof(DirectX::XMMATRIX);
        srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

        device->CreateShaderResourceView(m_dummyBoneBuffer.Get(), &srvDesc, m_dummyBoneSrvCpuHandle);
    }
}

ID3D12PipelineState* PSOManager::GetPSO(CMaterial* material, ID3D12RootSignature* rootSig)
{
    if (!material) return nullptr;

    // ?L???b?V???L?[??? (?V?F?[?_?[?t?@?C?? + ?G???g?? + ?u?????h???[?h)
    const std::string& vsEntry = material->GetVsEntry();
    std::wstring key = material->GetShaderFile() + L"_" + 
                       std::wstring(vsEntry.begin(), vsEntry.end()) + L"_" + 
                       std::to_wstring(static_cast<int>(material->GetBlendMode()));

    // ?L???b?V???q?b?g
    if (m_psoCache.find(key) != m_psoCache.end())
    {
        return m_psoCache[key].Get();
    }

    // ????????
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

    // ?u?????h???[?h??????????E
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
        depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // Z??????????E
        depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        psoBuilder.SetBlendState(blendDesc).SetDepthStencilState(depthDesc);
    }

    ComPtr<ID3D12PipelineState> newPso;
    psoBuilder.Build(device, &newPso);
    
    m_psoCache[key] = newPso;
    return newPso.Get();
}
