#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

// ========================================================
// RootSignatureBuilder
// ========================================================
class RootSignatureBuilder
{
public:
    RootSignatureBuilder();

    void AddConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility);
    void AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE rangeType, UINT numDescriptors, UINT baseShaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility);
    void AddConstantBufferView(UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visibility);
    void AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& sampler);
    void SetFlags(D3D12_ROOT_SIGNATURE_FLAGS flags) { m_flags = flags; }

    bool Build(ID3D12Device* device, ID3D12RootSignature** ppRootSig);

private:
    std::vector<D3D12_ROOT_PARAMETER> m_parameters;
    std::vector<std::vector<D3D12_DESCRIPTOR_RANGE>> m_descriptorRanges; 
    std::vector<D3D12_STATIC_SAMPLER_DESC> m_samplers;
    D3D12_ROOT_SIGNATURE_FLAGS m_flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
};


// ========================================================
// PSOBuilder (Graphics)
// ========================================================
class PSOBuilder
{
public:
    PSOBuilder();

    PSOBuilder& SetRootSignature(ID3D12RootSignature* rootSig);
    PSOBuilder& SetInputLayout(const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs, UINT NumElements);
    PSOBuilder& SetShaders(D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps);
    PSOBuilder& SetBlendState(const D3D12_BLEND_DESC& blendDesc);
    PSOBuilder& SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterDesc);
    PSOBuilder& SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthDesc);
    PSOBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);
    PSOBuilder& SetRenderTargetFormats(UINT numRenderTargets, const DXGI_FORMAT* rtvFormats, DXGI_FORMAT dsvFormat);

    PSOBuilder& SetDefaultRasterizer();
    PSOBuilder& SetDefaultDepthStencil();
    PSOBuilder& SetDefaultBlendState();

    bool Build(ID3D12Device* device, ID3D12PipelineState** ppPipelineState);

private:
    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
};

// ========================================================
// ComputePSOBuilder
// ========================================================
class ComputePSOBuilder
{
public:
    ComputePSOBuilder();

    ComputePSOBuilder& SetRootSignature(ID3D12RootSignature* rootSig);
    ComputePSOBuilder& SetComputeShader(D3D12_SHADER_BYTECODE cs);

    bool Build(ID3D12Device* device, ID3D12PipelineState** ppPipelineState);

private:
    D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
};
