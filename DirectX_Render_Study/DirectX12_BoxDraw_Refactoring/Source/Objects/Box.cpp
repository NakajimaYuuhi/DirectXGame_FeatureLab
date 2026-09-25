//===== ?C???N???[?h =====
#include "Box.h"
#include "DX12Manager.h"

//----- ?V?F?[?_?[?R???p?C???p -----
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")


#include "BasicSettings.h"



//TODO:?\?????X????
struct Vertex
{
    float position[3];
    float color[4];
    float uv[2];
};






//???_?f?[?^???
//?l?p?`????C?A?E?g???X
//?O????????????????
Vertex vertices[] =
{
    //??????(?C???f?b?N?X)
        // ===== ?O?? (Z-) =====
    {{-0.5f,-0.5f,-0.5f},{1,0,0,1},{0,1}},
    {{-0.5f, 0.5f,-0.5f},{1,0,0,1},{0,0}},
    {{ 0.5f, 0.5f,-0.5f},{1,0,0,1},{1,0}},
    {{ 0.5f,-0.5f,-0.5f},{1,0,0,1},{1,1}},

    // ===== ?w?? (Z+) =====
    {{-0.5f,-0.5f,0.5f},{0,1,0,1},{1,1}},
    {{ 0.5f,-0.5f,0.5f},{0,1,0,1},{0,1}},
    {{ 0.5f, 0.5f,0.5f},{0,1,0,1},{0,0}},
    {{-0.5f, 0.5f,0.5f},{0,1,0,1},{1,0}},

    // ===== ?? (X-) =====
    {{-0.5f,-0.5f, 0.5f},{0,0,1,1},{0,1}},
    {{-0.5f, 0.5f, 0.5f},{0,0,1,1},{0,0}},
    {{-0.5f, 0.5f,-0.5f},{0,0,1,1},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{0,0,1,1},{1,1}},

    // ===== ?E (X+) =====
    {{0.5f,-0.5f,-0.5f},{1,1,0,1},{0,1}},
    {{0.5f, 0.5f,-0.5f},{1,1,0,1},{0,0}},
    {{0.5f, 0.5f, 0.5f},{1,1,0,1},{1,0}},
    {{0.5f,-0.5f, 0.5f},{1,1,0,1},{1,1}},

    // ===== ?? (Y+) =====
    {{-0.5f,0.5f,-0.5f},{1,0,1,1},{0,1}},
    {{-0.5f,0.5f, 0.5f},{1,0,1,1},{0,0}},
    {{ 0.5f,0.5f, 0.5f},{1,0,1,1},{1,0}},
    {{ 0.5f,0.5f,-0.5f},{1,0,1,1},{1,1}},

    // ===== ?? (Y-) =====
    {{-0.5f,-0.5f, 0.5f},{0,1,1,1},{1,0}},
    {{-0.5f,-0.5f,-0.5f},{0,1,1,1},{1,1}},
    {{ 0.5f,-0.5f,-0.5f},{0,1,1,1},{0,1}},
    {{ 0.5f,-0.5f, 0.5f},{0,1,1,1},{0,0}},
};

uint16_t indices[] =
{
    0,1,2, 0,2,3,        // ?O
    4,5,6, 4,6,7,        // ?w
    8,9,10, 8,10,11,     // ??
    12,13,14, 12,14,15,  // ?E
    16,17,18, 16,18,19,  // ??
    20,21,22, 20,22,23   // ??
};

CBox::CBox()
{
	//Initialize();
}

//===== ???\?b?h??` =====
void CBox::Initialize(ID3D12Device* _Device)
{


	//----- ?V?F?[?_?[?R???p?C?? -----
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;
    ComPtr<ID3DBlob> errorBlob;

    HRESULT hr;

    //???_?V?F?[?_??R???p?C??
    hr = D3DCompileFromFile(
        L"Assets/Shader/Triangle.hlsl",
        nullptr,
        nullptr,
        "VSMain",
        "vs_5_0",
        0,
        0,
        &vertexShader,
        &errorBlob
    );

	//?s?N?Z???V?F?[?_??R???p?C??
    hr = D3DCompileFromFile(
        L"Assets/Shader/Triangle.hlsl",
        nullptr,
        nullptr,
        "PSMain",
        "ps_5_0",
        0,
        0,
        &pixelShader,
        &errorBlob
    );

    //----- ???[?g?V?O?l?`?? -----
    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    //WVP????p?????[?^????
    D3D12_ROOT_PARAMETER rootParam{};
    rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParam.Descriptor.ShaderRegister = 0; // b0
    rootParam.Descriptor.RegisterSpace = 0;
    rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;


    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc{};
    rootSignatureDesc.NumParameters = 1;
    rootSignatureDesc.pParameters = &rootParam;    //?p?????[?^??????????X
    rootSignatureDesc.NumStaticSamplers = 0;
    rootSignatureDesc.pStaticSamplers = nullptr;
    rootSignatureDesc.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	//???[?g?V?O?l?`????V???A???C?Y
    D3D12SerializeRootSignature(
        &rootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signature,
        &error
    );

	//???[?g?V?O?l?`?????
    hr = _Device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&m_rootSignature)
    );

    //???_???C?A?E?g???
    //0,12,28???}?W?b?N?i???o?[
    D3D12_INPUT_ELEMENT_DESC inputLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },

        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 28,
          D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    //PSO????
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
    psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };

    psoDesc.InputLayout.pInputElementDescs = inputLayout;
    psoDesc.InputLayout.NumElements = _countof(inputLayout);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;

    psoDesc.SampleDesc.Count = 1;
    psoDesc.SampleMask = UINT_MAX;

	//???X?^???C?U?[?X?e?[?g????
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


    //??????????H
    //psoDesc.DepthStencilState.DepthEnable = FALSE;
    //psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    //psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    //psoDesc.DepthStencilState.StencilEnable = FALSE;
    
    
    //----- ?[?x?o?b?t?@???\?[?X?? -----
   
    
	//----- DepthStencilState???? -----
    D3D12_DEPTH_STENCIL_DESC depthDesc = {};
    depthDesc.DepthEnable = TRUE;
    depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    depthDesc.StencilEnable = FALSE;

    psoDesc.DepthStencilState = depthDesc;

    // ?????d?v
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.NumRenderTargets = 1;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    psoDesc.pRootSignature = m_rootSignature.Get();


    //GraphicsPipelineState???
    hr = _Device->CreateGraphicsPipelineState(
        &psoDesc,
        IID_PPV_ARGS(&m_pipelineState)
    );

    if (FAILED(hr))
    {
        MessageBoxA(nullptr, "PSO creation failed", "Error", MB_OK);
    }


	//----- ???_?o?b?t?@??? -----
    //?T?C?Y?v?Z
    UINT vertexBufferSize = sizeof(vertices);

    //???\?[?X???iUploadHeap?j
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = vertexBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    _Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_vertexBuffer)
    );

	//???_?f?[?^??o?b?t?@??R?s?[
    void* mappedData = nullptr;
    m_vertexBuffer->Map(0, nullptr, &mappedData);
    memcpy(mappedData, vertices, vertexBufferSize);
    m_vertexBuffer->Unmap(0, nullptr);

	//???_?o?b?t?@?r???[????
    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = vertexBufferSize;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);


	//----- ???o?b?t?@??? -----
    //???o?b?t?@??T?C?Y??256?o?C?g??{???????
    UINT constantBufferSize =
        (sizeof(ConstantBufferData) + 255) & ~255;

	//UnloadHeap????\?[?X??
    //D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

    //D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Width = constantBufferSize;
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    _Device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );

    m_constantBuffer->Map(0, nullptr, (void**)&m_cbData);


	//----- ?C???f?b?N?X?o?b?t?@??? -----
	//?T?C?Y?v?Z
    const UINT indexBufferSize = sizeof(indices);

	//???\?[?X???iUploadHeap?j
    D3D12_HEAP_PROPERTIES heapProps2 = {};
    heapProps2.Type = D3D12_HEAP_TYPE_UPLOAD;
    heapProps2.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps2.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heapProps2.CreationNodeMask = 1;
    heapProps2.VisibleNodeMask = 1;

    D3D12_RESOURCE_DESC resourceDesc2 = {};
    resourceDesc2.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc2.Alignment = 0;
    resourceDesc2.Width = indexBufferSize;
    resourceDesc2.Height = 1;
    resourceDesc2.DepthOrArraySize = 1;
    resourceDesc2.MipLevels = 1;
    resourceDesc2.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc2.SampleDesc.Count = 1;
    resourceDesc2.SampleDesc.Quality = 0;
    resourceDesc2.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc2.Flags = D3D12_RESOURCE_FLAG_NONE;

    _Device->CreateCommittedResource(
        &heapProps2,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc2,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_indexBuffer)
    );

	//?C???f?b?N?X?f?[?^??o?b?t?@??R?s?[
    uint8_t* mappedData2 = nullptr;
    m_indexBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData2));
    memcpy(mappedData2, indices, indexBufferSize);
    m_indexBuffer->Unmap(0, nullptr);

	//?C???f?b?N?X?o?b?t?@?r???[????
    m_indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
    m_indexBufferView.SizeInBytes = sizeof(indices);
    m_indexBufferView.Format = DXGI_FORMAT_R16_UINT; // uint16_t?????


    

}

void CBox::Draw(ID3D12GraphicsCommandList* _CommandList)
{
    //?s???
    DirectX::XMMATRIX world =
        DirectX::XMMatrixScaling(
            m_Scale.x,
            m_Scale.y,
            m_Scale.z)
        *
        DirectX::XMMatrixTranslation(
            m_Position.x,
            m_Position.y,
            m_Position.z);

	//?r???[??v???W?F?N?V??????DX12Manager????ï
	DirectX::XMMATRIX view = DX12Manager::GetInstance().GetView();
    DirectX::XMMATRIX proj = DX12Manager::GetInstance().GetProj();


    DirectX::XMMATRIX wvp = world * view * proj;

    m_cbData->WVP = XMMatrixTranspose(wvp);



    ConstantBufferData* cbData = nullptr;
    m_constantBuffer->Map(0, nullptr, (void**)&cbData);
    cbData->WVP = XMMatrixTranspose(wvp);
    m_constantBuffer->Unmap(0, nullptr);


    _CommandList->SetPipelineState(m_pipelineState.Get());
    _CommandList->SetGraphicsRootSignature(m_rootSignature.Get());
    
    _CommandList->SetGraphicsRootConstantBufferView(
        0,
        m_constantBuffer->GetGPUVirtualAddress()
    );

    _CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    //?C???f?b?N?X?p???X
    _CommandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    _CommandList->IASetIndexBuffer(&m_indexBufferView);
    _CommandList->DrawIndexedInstanced(sizeof(indices)/sizeof(indices[0]), 1, 0, 0, 0);
}
