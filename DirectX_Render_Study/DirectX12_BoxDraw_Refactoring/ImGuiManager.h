#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>

class CImGuiManager
{
public:
    CImGuiManager();
    ~CImGuiManager();

    //‰Šú‰»
    bool Initialize(
        HWND hwnd,
        ID3D12Device* pDevice,
        ID3D12DescriptorHeap* pSrvHeap,
        int frameCount
    );

    //•`‰æˆ—
    void BeginFrame();   // NewFrameˆ—
    void EndFrame();     // RenderŒÄ‚Ño‚µ‘OŒã
    void Render(ID3D12GraphicsCommandList* pCommandList);

    //Œãˆ—
    void Finalize();

private:
    HWND m_hWnd;

    ID3D12Device* m_pDevice;
    ID3D12DescriptorHeap* m_pSrvHeap;
    int m_FrameCount;

    bool m_IsInitialized = false;
};

