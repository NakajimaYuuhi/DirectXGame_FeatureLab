//===== インクルード =====
#include "DX12Manager.h"

#include <d3dcompiler.h>
#include <iostream>

//===== メソッド定義 =====

//インスタンス取得
CDX12Manager& CDX12Manager::GetInstance()
{
    static CDX12Manager instance;
    return instance;
}

// <初期化、終了処理>
//初期化処理
bool CDX12Manager::Initialize(HWND hwnd)
{
    HRESULT hr;

#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

    // DXGI Factory 作成
    hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_factory));
    if (FAILED(hr))
        return false;

    // アダプタ取得
    ComPtr<IDXGIAdapter1> adapter;

    for (UINT i = 0;
        m_factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND;
        ++i)
    {
        DXGI_ADAPTER_DESC1 desc;
        adapter->GetDesc1(&desc);

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        break;
    }

    // デバイス作成
    hr = D3D12CreateDevice(
        adapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&m_device)
    );

    if (FAILED(hr))
        return false;

    // コマンドキュー作成
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    hr = m_device->CreateCommandQueue(
        &queueDesc,
        IID_PPV_ARGS(&m_commandQueue)
    );

    if (FAILED(hr))
        return false;

    return true;
}

//終了処理
void CDX12Manager::Finalize()
{
    m_commandQueue.Reset();
    m_device.Reset();
    m_factory.Reset();
}