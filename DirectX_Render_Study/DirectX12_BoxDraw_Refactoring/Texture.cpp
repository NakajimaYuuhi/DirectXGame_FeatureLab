#include "Texture.h"
#include "DX12Manager.h"

bool CTexture::LoadTexture(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const wchar_t* filePath, int srvIndex)
{
    // WIC系 (PNG/JPG/BMP)
    HRESULT hr = DirectX::LoadFromWICFile(
        filePath,
        DirectX::WIC_FLAGS_NONE,
        &metadata,
        scratch
    );
    if (FAILED(hr)) return false;

    const DirectX::Image* img = scratch.GetImage(0, 0, 0);

    // --- GPU用のテクスチャ
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Width = metadata.width;
    texDesc.Height = (UINT)metadata.height;
    texDesc.DepthOrArraySize = (UINT16)metadata.arraySize;
    texDesc.MipLevels = (UINT16)metadata.mipLevels;
    texDesc.Format = metadata.format;
    texDesc.SampleDesc.Count = 1;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);

    hr = device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&texture)
    );
    if (FAILED(hr)) return false;

    // --- Upload heap
    UINT64 uploadSize = GetRequiredIntermediateSize(texture.Get(), 0, metadata.mipLevels);

    CD3DX12_HEAP_PROPERTIES uploadHeapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC bufferDesc =
        CD3DX12_RESOURCE_DESC::Buffer(uploadSize);

    hr = device->CreateCommittedResource(
        &uploadHeapProps,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&uploadHeap)
    );
    if (FAILED(hr)) return false;

    // --- サブリソースのコピー
    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = img->pixels;
    textureData.RowPitch = img->rowPitch;
    textureData.SlicePitch = img->slicePitch;

    UpdateSubresources(cmdList, texture.Get(), uploadHeap.Get(), 0, 0, metadata.mipLevels, &textureData);

    // 最後にシェーダーで読めるように
    CD3DX12_RESOURCE_BARRIER barrier =
        CD3DX12_RESOURCE_BARRIER::Transition(
            texture.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
        );
    cmdList->ResourceBarrier(1, &barrier);

    return true;
}

void CTexture::CreateSRV(ID3D12Device* device)
{
    CDX12Manager& dx12 = CDX12Manager::GetInstance();

    int index = dx12.AllocsrvNextIndex();
    m_srvIndex = index;


    // ---- 2. SRV 設定 ----
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = metadata.format; // 読み込んだ画像のフォーマット
    //srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;

    srvDesc.Texture2D.MipLevels = texture->GetDesc().MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;


    auto cpuHandle = dx12.GetCpuSrvHandle(m_srvIndex);


    device->CreateShaderResourceView(texture.Get(), &srvDesc, cpuHandle);


    m_gpuHandle = dx12.GetGpuSrvHandle(m_srvIndex);

}
