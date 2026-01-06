#pragma once
#include <d3d11.h>

class DirectX11
{
public:
    DirectX11();
    ~DirectX11();

    bool Init(HWND hWnd, int width, int height);
    void BeginFrame(float r, float g, float b, float a);
    void EndFrame();

private:
    void Cleanup();

public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* Context = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11RenderTargetView* RTV = nullptr;
};
