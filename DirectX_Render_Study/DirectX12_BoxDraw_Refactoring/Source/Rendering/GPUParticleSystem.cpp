#include "GPUParticleSystem.h"
#include "PSOBuilder.h"
#include "Shader.h"
#include "DX12Manager.h"
#include "d3dx12.h"
#include <algorithm>

GPUParticleSystem::GPUParticleSystem()
{
}

GPUParticleSystem::~GPUParticleSystem()
{
    if (m_uavCPU.ptr != 0)
    {
        DX12Manager::GetInstance().GetSRVAllocator()->Free(m_uavCPU, m_uavGPU);
        m_uavCPU = {};
        m_uavGPU = {};
    }
    if (m_srvCPU.ptr != 0)
    {
        DX12Manager::GetInstance().GetSRVAllocator()->Free(m_srvCPU, m_srvGPU);
        m_srvCPU = {};
        m_srvGPU = {};
    }
}

bool GPUParticleSystem::Init(ID3D12Device* device, UINT maxParticles)
{
    m_maxParticles = maxParticles;

    if (!CreateBuffers(device)) return false;
    if (!CreateSimulationPipeline(device)) return false;
    if (!CreateRenderPipeline(device)) return false;

    return true;
}

bool GPUParticleSystem::CreateBuffers(ID3D12Device* device)
{
    UINT64 bufferSize = sizeof(GPUParticle) * m_maxParticles;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(
        bufferSize,
        D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    HRESULT hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COMMON,
        nullptr,
        IID_PPV_ARGS(&m_pParticleBuffer)
    );
    if (FAILED(hr))
    {
        OutputDebugStringA("Failed to create GPUParticle StructuredBuffer!\n");
        return false;
    }
    m_currentState = D3D12_RESOURCE_STATE_COMMON;

    // Allocate UAV Descriptor
    DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_uavCPU, &m_uavGPU);

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_UNKNOWN;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
    uavDesc.Buffer.FirstElement = 0;
    uavDesc.Buffer.NumElements = m_maxParticles;
    uavDesc.Buffer.StructureByteStride = sizeof(GPUParticle);
    uavDesc.Buffer.CounterOffsetInBytes = 0;
    uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

    device->CreateUnorderedAccessView(m_pParticleBuffer.Get(), nullptr, &uavDesc, m_uavCPU);

    // Allocate SRV Descriptor
    DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_srvCPU, &m_srvGPU);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.FirstElement = 0;
    srvDesc.Buffer.NumElements = m_maxParticles;
    srvDesc.Buffer.StructureByteStride = sizeof(GPUParticle);
    srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

    device->CreateShaderResourceView(m_pParticleBuffer.Get(), &srvDesc, m_srvCPU);

    return true;
}

bool GPUParticleSystem::CreateSimulationPipeline(ID3D12Device* device)
{
    // Simulation RootSignature
    // Param 0: 32-bit constants (b0) - 16 DWORDs = 64 bytes
    // Param 1: UAV table (u0)
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_NONE);
    rootSigBuilder.AddConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootSigBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    if (!rootSigBuilder.Build(device, m_pCSRootSignature.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build GPUParticle CS RootSignature!\n");
        return false;
    }

    // Compile CS
    Shader csShader;
    if (!csShader.CompileFromFile(L"Assets/Shader/GPUParticleCS.hlsl", "CSMain", "cs_5_0"))
    {
        OutputDebugStringA("Failed to compile GPUParticleCS.hlsl!\n");
        return false;
    }

    ComputePSOBuilder psoBuilder;
    psoBuilder.SetRootSignature(m_pCSRootSignature.Get());
    psoBuilder.SetComputeShader(csShader.GetBytecode());

    if (!psoBuilder.Build(device, m_pCSPipelineState.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build GPUParticle Compute PSO!\n");
        return false;
    }

    return true;
}

bool GPUParticleSystem::CreateRenderPipeline(ID3D12Device* device)
{
    // Render RootSignature
    // Param 0: 32-bit constants (b0) - Camera info (ViewProj 16 floats, Right 3 floats + pad, Up 3 floats + pad = 24 DWORDs)
    // Param 1: SRV table (t0) - StructuredBuffer<GPUParticle>
    RootSignatureBuilder rootSigBuilder;
    rootSigBuilder.SetFlags(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
    rootSigBuilder.AddConstants(24, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootSigBuilder.AddDescriptorTable(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    if (!rootSigBuilder.Build(device, m_pRenderRootSignature.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build GPUParticle Render RootSignature!\n");
        return false;
    }

    Shader vsShader, psShader;
    if (!vsShader.CompileFromFile(L"Assets/Shader/GPUParticleRender.hlsl", "VSMain", "vs_5_0"))
    {
        OutputDebugStringA("Failed to compile GPUParticleRender.hlsl VS!\n");
        return false;
    }
    if (!psShader.CompileFromFile(L"Assets/Shader/GPUParticleRender.hlsl", "PSMain", "ps_5_0"))
    {
        OutputDebugStringA("Failed to compile GPUParticleRender.hlsl PS!\n");
        return false;
    }

    PSOBuilder psoBuilder;
    psoBuilder.SetRootSignature(m_pRenderRootSignature.Get());
    psoBuilder.SetShaders(vsShader.GetBytecode(), psShader.GetBytecode());

    // Additive Blend
    D3D12_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
    blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    psoBuilder.SetBlendState(blendDesc);

    // Depth Test ON, Depth Write OFF
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    depthDesc.StencilEnable = FALSE;
    psoBuilder.SetDepthStencilState(depthDesc);

    // Rasterizer: Cull None
    D3D12_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterDesc.DepthClipEnable = TRUE;
    psoBuilder.SetRasterizerState(rasterDesc);

    psoBuilder.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

    DXGI_FORMAT rtvFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
    psoBuilder.SetRenderTargetFormats(1, rtvFormats, DXGI_FORMAT_D32_FLOAT);

    if (!psoBuilder.Build(device, m_pRenderPipelineState.ReleaseAndGetAddressOf()))
    {
        OutputDebugStringA("Failed to build GPUParticle Render PSO!\n");
        return false;
    }

    return true;
}

void GPUParticleSystem::UpdateAndSimulate(ID3D12GraphicsCommandList* cmdList, float deltaTime)
{
    if (!m_isEnabled || !m_pParticleBuffer || !m_pCSPipelineState) return;

    m_totalTime += deltaTime;

    // Calculate spawn count this frame
    m_emitAccumulator += m_emitRate * deltaTime;
    UINT emitCountThisFrame = static_cast<UINT>(m_emitAccumulator);
    if (emitCountThisFrame > 0)
    {
        m_emitAccumulator -= static_cast<float>(emitCountThisFrame);
    }

    // Transition buffer to UNORDERED_ACCESS
    if (m_currentState != D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pParticleBuffer.Get(),
            m_currentState,
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS
        );
        cmdList->ResourceBarrier(1, &barrier);
        m_currentState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    }

    cmdList->SetPipelineState(m_pCSPipelineState.Get());
    cmdList->SetComputeRootSignature(m_pCSRootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    struct SimConstants
    {
        float  deltaTime;
        float  totalTime;
        UINT   maxParticles;
        UINT   emitCountThisFrame;

        DirectX::XMFLOAT3 emitterPos;
        float  particleScale;

        DirectX::XMFLOAT3 gravity;
        float  initialSpeed;

        DirectX::XMFLOAT4 particleColor;
    } constants;

    constants.deltaTime = deltaTime;
    constants.totalTime = m_totalTime;
    constants.maxParticles = m_maxParticles;
    constants.emitCountThisFrame = emitCountThisFrame;
    constants.emitterPos = m_emitterPosition;
    constants.particleScale = m_particleScale;
    constants.gravity = m_gravity;
    constants.initialSpeed = m_initialSpeed;
    constants.particleColor = m_particleColor;

    cmdList->SetComputeRoot32BitConstants(0, sizeof(SimConstants) / sizeof(DWORD), &constants, 0);
    cmdList->SetComputeRootDescriptorTable(1, m_uavGPU);

    // Dispatch (threads per group: 64)
    UINT threadGroupX = (m_maxParticles + 63) / 64;
    cmdList->Dispatch(threadGroupX, 1, 1);

    // UAV Barrier
    CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(m_pParticleBuffer.Get());
    cmdList->ResourceBarrier(1, &uavBarrier);
}

void GPUParticleSystem::Render(ID3D12GraphicsCommandList* cmdList, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj)
{
    if (!m_isEnabled || !m_pParticleBuffer || !m_pRenderPipelineState) return;

    // Transition buffer to PIXEL_SHADER_RESOURCE | NON_PIXEL_SHADER_RESOURCE
    D3D12_RESOURCE_STATES renderState = D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    if (m_currentState != renderState)
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            m_pParticleBuffer.Get(),
            m_currentState,
            renderState
        );
        cmdList->ResourceBarrier(1, &barrier);
        m_currentState = renderState;
    }

    cmdList->SetPipelineState(m_pRenderPipelineState.Get());
    cmdList->SetGraphicsRootSignature(m_pRenderRootSignature.Get());

    ID3D12DescriptorHeap* ppHeaps[] = { DX12Manager::GetInstance().GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

    // Extract Camera Right and Up vectors from View Matrix (inverse of View)
    DirectX::XMVECTOR det;
    DirectX::XMMATRIX invView = DirectX::XMMatrixInverse(&det, view);
    DirectX::XMFLOAT3 camRight = { invView.r[0].m128_f32[0], invView.r[0].m128_f32[1], invView.r[0].m128_f32[2] };
    DirectX::XMFLOAT3 camUp    = { invView.r[1].m128_f32[0], invView.r[1].m128_f32[1], invView.r[1].m128_f32[2] };

    DirectX::XMMATRIX viewProj = DirectX::XMMatrixMultiply(view, proj);

    struct CameraConstants
    {
        DirectX::XMFLOAT4X4 viewProj;
        DirectX::XMFLOAT3   cameraRight;
        float               pad0;
        DirectX::XMFLOAT3   cameraUp;
        float               pad1;
    } camData;

    DirectX::XMStoreFloat4x4(&camData.viewProj, DirectX::XMMatrixTranspose(viewProj));
    camData.cameraRight = camRight;
    camData.pad0 = 0.0f;
    camData.cameraUp = camUp;
    camData.pad1 = 0.0f;

    cmdList->SetGraphicsRoot32BitConstants(0, sizeof(CameraConstants) / sizeof(DWORD), &camData, 0);
    cmdList->SetGraphicsRootDescriptorTable(1, m_srvGPU);

    cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // 6 vertices per quad, m_maxParticles instances
    cmdList->DrawInstanced(6, m_maxParticles, 0, 0);
}

void GPUParticleSystem::ResetParticles(ID3D12GraphicsCommandList* cmdList)
{
    // Simply zero or reset simulation time
    m_totalTime = 0.0f;
    m_emitAccumulator = 0.0f;
}
