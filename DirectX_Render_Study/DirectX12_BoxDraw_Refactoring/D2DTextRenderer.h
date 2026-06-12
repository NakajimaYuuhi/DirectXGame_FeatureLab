#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d11on12.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl.h>
#include <vector>
#include <string>

using Microsoft::WRL::ComPtr;

class D2DTextRenderer
{
public:
    static D2DTextRenderer& GetInstance();

    bool Initialize(ID3D12Device* d3d12Device, ID3D12CommandQueue* commandQueue, IDXGISwapChain4* swapChain, UINT width, UINT height, UINT frameBufferCount);
    void Finalize();

    void Resize(UINT width, UINT height, IDXGISwapChain4* swapChain);

    // テキスト描画のリクエストを登録
    void DrawTextStr(const std::wstring& text, float x, float y, float fontSize = 24.0f, D2D1::ColorF color = D2D1::ColorF::White, const std::wstring& fontFamily = L"Meiryo");

    // 描画実行（DX12のレンダーターゲットに対して）
    void Render(UINT frameIndex);

private:
    D2DTextRenderer() = default;
    ~D2DTextRenderer() = default;

    D2DTextRenderer(const D2DTextRenderer&) = delete;
    D2DTextRenderer& operator=(const D2DTextRenderer&) = delete;

    bool CreateSizeDependentResources(UINT width, UINT height, IDXGISwapChain4* swapChain);
    void ReleaseSizeDependentResources();

private:
    // D3D11オン12 関連
    ComPtr<ID3D11Device> m_d3d11Device;
    ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
    ComPtr<ID3D11On12Device> m_d3d11On12Device;

    // D2D 関連
    ComPtr<ID2D1Factory3> m_d2dFactory;
    ComPtr<ID2D1Device2> m_d2dDevice;
    ComPtr<ID2D1DeviceContext2> m_d2dContext;

    // DWrite 関連
    ComPtr<IDWriteFactory> m_dwriteFactory;
    ComPtr<IDWriteTextFormat> m_defaultTextFormat;

    // バックバッファ関連のリソース
    struct FrameResources
    {
        ComPtr<ID3D11Resource> wrappedResource;
        ComPtr<ID2D1Bitmap1> d2dBitmap;
    };
    std::vector<FrameResources> m_frameResources;
    UINT m_frameBufferCount = 0;
    UINT m_width = 0;
    UINT m_height = 0;

    // 描画テキスト情報
    struct TextInfo
    {
        std::wstring text;
        float x, y;
        float fontSize;
        D2D1::ColorF color;
        std::wstring fontFamily;
    };
    std::vector<TextInfo> m_textQueue;
};
