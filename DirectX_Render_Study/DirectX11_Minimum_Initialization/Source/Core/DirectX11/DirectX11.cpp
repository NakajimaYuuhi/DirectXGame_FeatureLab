#include "DirectX11.h"

DirectX11::DirectX11() {}
DirectX11::~DirectX11() { Cleanup(); }

bool DirectX11::Init(HWND hWnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &scd,
        &SwapChain,
        &Device,
        nullptr,
        &Context
    );

    if (FAILED(hr)) return false;

    ID3D11Texture2D* backBuffer = nullptr;
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    Device->CreateRenderTargetView(backBuffer, nullptr, &RTV);
    backBuffer->Release();

    return true;
}

void DirectX11::BeginFrame(float r, float g, float b, float a)
{
    float col[4] = { r, g, b, a };
    Context->OMSetRenderTargets(1, &RTV, nullptr);
    Context->ClearRenderTargetView(RTV, col);
}

void DirectX11::EndFrame()
{
    SwapChain->Present(1, 0);
}

void DirectX11::Cleanup()
{
    if (RTV) RTV->Release();
    if (SwapChain) SwapChain->Release();
    if (Context) Context->Release();
    if (Device) Device->Release();
}
