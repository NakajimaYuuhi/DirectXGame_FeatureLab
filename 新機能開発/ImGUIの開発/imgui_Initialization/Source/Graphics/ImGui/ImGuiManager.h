#pragma once

#include <Windows.h>
#include <d3d11.h>

class CImGuiManager
{
public:
    CImGuiManager();
    ~CImGuiManager();

    bool Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context);
    void Begin();
    void End();
    void Shutdown();
};

