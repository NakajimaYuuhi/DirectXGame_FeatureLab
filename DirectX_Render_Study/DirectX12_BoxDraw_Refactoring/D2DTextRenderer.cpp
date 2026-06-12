#include "D2DTextRenderer.h"
#include <d3d11on12.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

D2DTextRenderer& D2DTextRenderer::GetInstance()
{
    static D2DTextRenderer instance;
    return instance;
}

bool D2DTextRenderer::Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commandQueue, IDXGISwapChain4* swapChain, UINT width, UINT height, UINT frameBufferCount)
{
    m_frameBufferCount = frameBufferCount;
    m_width = width;
    m_height = height;

    // 1. D3D11オン12デバイスの作成
    UINT d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    d3d11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    IUnknown* queueInterface = commandQueue;

    HRESULT hr = D3D11On12CreateDevice(
        d3d12Device,
        d3d11DeviceFlags,
        featureLevels,
        _countof(featureLevels),
        &queueInterface,
        1,
        0, // デフォルトノード
        &m_d3d11Device,
        &m_d3d11DeviceContext,
        nullptr
    );

    if (FAILED(hr))
    {
        OutputDebugStringA("D3D11On12CreateDevice failed\n");
        return false;
    }

    hr = m_d3d11Device.As(&m_d3d11On12Device);
    if (FAILED(hr))
    {
        return false;
    }

    // 2. Direct2D デバイスとコンテキストの作成
    ComPtr<IDXGIDevice3> dxgiDevice;
    hr = m_d3d11Device.As(&dxgiDevice);
    if (FAILED(hr)) return false;

    D2D1_FACTORY_OPTIONS options = {};
#if defined(_DEBUG)
    options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &options, &m_d2dFactory);
    if (FAILED(hr)) return false;

    hr = m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice);
    if (FAILED(hr)) return false;

    hr = m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dContext);
    if (FAILED(hr)) return false;

    // 3. DirectWrite ファクトリの作成
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dwriteFactory);
    if (FAILED(hr)) return false;

    // デフォルトテキストフォーマットの作成
    hr = m_dwriteFactory->CreateTextFormat(
        L"Meiryo",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        24.0f,
        L"ja-jp",
        &m_defaultTextFormat
    );
    if (FAILED(hr)) return false;

    // 4. サイズ依存リソース（レンダーターゲットビットマップ）の作成
    return CreateSizeDependentResources(width, height, swapChain);
}

void D2DTextRenderer::Finalize()
{
    ReleaseSizeDependentResources();
    m_defaultTextFormat.Reset();
    m_dwriteFactory.Reset();
    m_d2dContext.Reset();
    m_d2dDevice.Reset();
    m_d2dFactory.Reset();
    m_d3d11On12Device.Reset();
    m_d3d11DeviceContext.Reset();
    m_d3d11Device.Reset();
}

bool D2DTextRenderer::CreateSizeDependentResources(UINT width, UINT height, IDXGISwapChain4* swapChain)
{
    m_width = width;
    m_height = height;
    m_frameResources.resize(m_frameBufferCount);

    // テキスト解像度（DPI）の設定
    float dpiX = 96.0f;
    float dpiY = 96.0f;
    // DPI情報を設定
    m_d2dContext->SetDpi(dpiX, dpiY);

    D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        dpiX,
        dpiY
    );

    for (UINT i = 0; i < m_frameBufferCount; ++i)
    {
        ComPtr<ID3D12Resource> backBuffer;
        HRESULT hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;

        // D3D11のリソースとしてラップする
        D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
        hr = m_d3d11On12Device->CreateWrappedResource(
            backBuffer.Get(),
            &d3d11Flags,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT,
            IID_PPV_ARGS(&m_frameResources[i].wrappedResource)
        );

        if (FAILED(hr)) return false;

        // DXGI サーフェスを取得
        ComPtr<IDXGISurface> dxgiSurface;
        hr = m_frameResources[i].wrappedResource.As(&dxgiSurface);
        if (FAILED(hr)) return false;

        // D2D ビットマップを作成
        hr = m_d2dContext->CreateBitmapFromDxgiSurface(
            dxgiSurface.Get(),
            &bitmapProperties,
            &m_frameResources[i].d2dBitmap
        );

        if (FAILED(hr)) return false;
    }

    return true;
}

void D2DTextRenderer::ReleaseSizeDependentResources()
{
    m_d2dContext->SetTarget(nullptr);
    for (auto& res : m_frameResources)
    {
        res.d2dBitmap.Reset();
        res.wrappedResource.Reset();
    }
    m_frameResources.clear();
}

void D2DTextRenderer::Resize(UINT width, UINT height, IDXGISwapChain4* swapChain)
{
    ReleaseSizeDependentResources();
    CreateSizeDependentResources(width, height, swapChain);
}

void D2DTextRenderer::DrawTextStr(const std::wstring& text, float x, float y, float fontSize, D2D1::ColorF color, const std::wstring& fontFamily)
{
    m_textQueue.push_back({ text, x, y, fontSize, color, fontFamily });
}

void D2DTextRenderer::Render(UINT frameIndex)
{
    if (frameIndex >= m_frameResources.size()) return;

    ID3D11Resource* wrappedResource = m_frameResources[frameIndex].wrappedResource.Get();

    // 1. ラップされたリソースを取得 (RENDER_TARGET に遷移)
    m_d3d11On12Device->AcquireWrappedResources(&wrappedResource, 1);

    // テキストがある場合のみ描画を行う
    if (!m_textQueue.empty())
    {
        // 2. レンダーターゲットの設定
        m_d2dContext->SetTarget(m_frameResources[frameIndex].d2dBitmap.Get());

        // 3. 描画開始
        m_d2dContext->BeginDraw();

        for (const auto& info : m_textQueue)
        {
            // 色ブラシの作成
            ComPtr<ID2D1SolidColorBrush> brush;
            m_d2dContext->CreateSolidColorBrush(info.color, &brush);

            // テキストフォーマットの作成
            ComPtr<IDWriteTextFormat> textFormat;
            if (info.fontSize == 24.0f && info.fontFamily == L"Meiryo")
            {
                textFormat = m_defaultTextFormat;
            }
            else
            {
                m_dwriteFactory->CreateTextFormat(
                    info.fontFamily.c_str(),
                    nullptr,
                    DWRITE_FONT_WEIGHT_NORMAL,
                    DWRITE_FONT_STYLE_NORMAL,
                    DWRITE_FONT_STRETCH_NORMAL,
                    info.fontSize,
                    L"ja-jp",
                    &textFormat
                );
            }

            if (textFormat && brush)
            {
                // 描画領域の設定
                D2D1_RECT_F rect = D2D1::RectF(info.x, info.y, (float)m_width, (float)m_height);
                m_d2dContext->DrawText(
                    info.text.c_str(),
                    (UINT32)info.text.length(),
                    textFormat.Get(),
                    rect,
                    brush.Get()
                );
            }
        }

        // 4. 描画終了
        m_d2dContext->EndDraw();
    }

    // 5. リソースを解放 (PRESENT に遷移)
    m_d3d11On12Device->ReleaseWrappedResources(&wrappedResource, 1);

    // 6. フラッシュ
    m_d3d11DeviceContext->Flush();

    // キューをクリア
    m_textQueue.clear();
}
