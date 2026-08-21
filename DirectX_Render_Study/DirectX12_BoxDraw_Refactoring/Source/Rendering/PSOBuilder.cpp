#include "PSOBuilder.h"

// ========================================================
// RootSignatureBuilder
// ========================================================

RootSignatureBuilder::RootSignatureBuilder()
{
}

void RootSignatureBuilder::AddConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    param.Constants.Num32BitValues = num32BitValues;
    param.Constants.ShaderRegister = shaderRegister;
    param.Constants.RegisterSpace = registerSpace;
    param.ShaderVisibility = visibility;
    m_parameters.push_back(param);
}

void RootSignatureBuilder::AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility)
{
    D3D12_DESCRIPTOR_RANGE range = {};
    range.RangeType = rangeType;
    range.NumDescriptors = numDescriptors;
    range.BaseShaderRegister = baseShaderRegister;
    range.RegisterSpace = registerSpace;
    range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    m_descriptorRanges.push_back({ range });

    D3D12_ROOT_PARAMETER param = {};
    param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    param.DescriptorTable.NumDescriptorRanges = 1;
    param.ShaderVisibility = visibility;

    m_parameters.push_back(param);
}

void RootSignatureBuilder::AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& sampler)
{
    m_samplers.push_back(sampler);
}

bool RootSignatureBuilder::Build(ID3D12Device* device, ID3D12RootSignature** ppRootSig)
{
    int rangeIndex = 0;
    for (auto& param : m_parameters)
    {
        if (param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        {
            param.DescriptorTable.pDescriptorRanges = m_descriptorRanges[rangeIndex].data();
            rangeIndex++;
        }
    }

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.NumParameters = static_cast<UINT>(m_parameters.size());
    rootSignatureDesc.pParameters = m_parameters.empty() ? nullptr : m_parameters.data();
    rootSignatureDesc.NumStaticSamplers = static_cast<UINT>(m_samplers.size());
    rootSignatureDesc.pStaticSamplers = m_samplers.empty() ? nullptr : m_samplers.data();
    rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
    if (FAILED(hr))
    {
        if (error)
        {
            OutputDebugStringA((char*)error->GetBufferPointer());
        }
        return false;
    }

    hr = device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(ppRootSig));
    return SUCCEEDED(hr);
}


// ========================================================
// PSOBuilder
// ========================================================

PSOBuilder::PSOBuilder()
{
    ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
    m_psoDesc.SampleMask = UINT_MAX;
    m_psoDesc.SampleDesc.Count = 1;
    
    SetDefaultRasterizer();
    SetDefaultDepthStencil();
    SetDefaultBlendState();
}

PSOBuilder& PSOBuilder::SetRootSignature(ID3D12RootSignature* rootSig)
{
    m_psoDesc.pRootSignature = rootSig;
    return *this;
}

PSOBuilder& PSOBuilder::SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements)
{
    m_psoDesc.InputLayout.pInputElementDescs = pInputElementDescs;
    m_psoDesc.InputLayout.NumElements = NumElements;
    return *this;
}

PSOBuilder& PSOBuilder::SetShaders(D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps)
{
    m_psoDesc.VS = vs;
    m_psoDesc.PS = ps;
    return *this;
}

PSOBuilder& PSOBuilder::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
{
    m_psoDesc.BlendState = blendDesc;
    return *this;
}

PSOBuilder& PSOBuilder::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc)
{
    m_psoDesc.RasterizerState = rasterDesc;
    return *this;
}

PSOBuilder& PSOBuilder::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthDesc)
{
    m_psoDesc.DepthStencilState = depthDesc;
    return *this;
}

PSOBuilder& PSOBuilder::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
{
    m_psoDesc.PrimitiveTopologyType = topologyType;
    return *this;
}

PSOBuilder& PSOBuilder::SetRenderTargetFormats(UINT numRenderTargets, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat)
{
    m_psoDesc.NumRenderTargets = numRenderTargets;
    for (UINT i = 0; i < numRenderTargets; ++i)
    {
        m_psoDesc.RTVFormats[i] = rtvFormats[i];
    }
    m_psoDesc.DSVFormat = dsvFormat;
    return *this;
}

PSOBuilder& PSOBuilder::SetDefaultRasterizer()
{
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
    
    m_psoDesc.RasterizerState = rasterDesc;
    return *this;
}

PSOBuilder& PSOBuilder::SetDefaultDepthStencil()
{
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;

    m_psoDesc.DepthStencilState = depthDesc;
    return *this;
}

PSOBuilder& PSOBuilder::SetDefaultBlendState()
{
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    
    D3D12_RENDER_TARGET_BLEND_DESC defaultRTBlend = {};
    defaultRTBlend.BlendEnable = FALSE;
    defaultRTBlend.LogicOpEnable = FALSE;
    defaultRTBlend.SrcBlend = D3D12_BLEND_ONE;
    defaultRTBlend.DestBlend = D3D12_BLEND_ZERO;
    defaultRTBlend.BlendOp = D3D12_BLEND_OP_ADD;
    defaultRTBlend.SrcBlendAlpha = D3D12_BLEND_ONE;
    defaultRTBlend.DestBlendAlpha = D3D12_BLEND_ZERO;
    defaultRTBlend.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    defaultRTBlend.LogicOp = D3D12_LOGIC_OP_NOOP;
    defaultRTBlend.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
        blendDesc.RenderTarget[i] = defaultRTBlend;

    m_psoDesc.BlendState = blendDesc;
    return *this;
}

bool PSOBuilder::Build(ID3D12Device* device, ID3D12PipelineState** ppPipelineState)
{
    HRESULT hr = device->CreateGraphicsPipelineState(&m_psoDesc, IID_PPV_ARGS(ppPipelineState));
    return SUCCEEDED(hr);
}
