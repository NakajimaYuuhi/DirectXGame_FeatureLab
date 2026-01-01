///////////////////////////////////////////
//main.cpp                               
//                                       
//  imguiの初期化プログラム               
//                                         
//  ～概要～                             
//    imguiを初期化する
//                                       
//  ～更新履歴～                         
//  2025/01/01 制作開始 -Nakajima Yuhi-  
//   
//                                       
///////////////////////////////////////////

#include "Window.h"
#include "DirectX11.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Window window;
    DirectX11 dx;

    const int WIDTH = 1280;
    const int HEIGHT = 720;

    if (!window.Create("DirectX11 Window", WIDTH, HEIGHT))
        return -1;

    if (!dx.Init(window.GetHWND(), WIDTH, HEIGHT))
        return -1;

    while (window.ProcessMessage())
    {
        dx.BeginFrame(0.1f, 0.2f, 0.3f, 1.0f);

        // --- ここに今後 ImGui や描画コードを書く ---

        dx.EndFrame();
    }

    return 0;
}

























////===== インクルード =====
//#include <iostream>
//
////===== 名前空間宣言 =====
//using namespace std;
//
////===== 定数・マクロ定義 =====
//
////===== 構造体定義 =====
//
////===== グローバル変数宣言 =====
//
////===== プロトタイプ宣言 =====
//
//
////===== DirectXのインクルード、グローバル宣言 =====
//#include <d3d11.h>
//#pragma comment(lib, "d3d11.lib")
//
//ID3D11Device* g_Device = nullptr;
//ID3D11DeviceContext* g_Context = nullptr;
//IDXGISwapChain* g_SwapChain = nullptr;
//ID3D11RenderTargetView* g_RTV = nullptr;
//
////===== DirectXの関数の宣言 =====
//bool InitDirectX11(HWND hWnd, int width, int height);
//void BeginDraw(float r, float g, float b, float a);
//
////===== DirectXの初期化に必要な変数宣言 =====
//float g_Color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//int   g_WindowSize[2]{ 1080, 1920 };
//
//
///// //////////////////////////
////main関数                  
////                          
////概　要:エントリーポイント 
////引　数:なし               
////戻り値:0                  
////////////////////////////////
//int main() {
//
//	//----- 変数宣言 -----
//
//	//----- 初期化処理 -----
//
//	//----- 前処理 -----
//
//	//----- 更新処理 -----
//
//	//----- 事後処理 -----
//	rewind(stdin);
//	getchar();
//	return 0;
//}
//
//
////===== DirectXの関数の定義 =====
//
////DirectXの初期化
//bool InitDirectX11(HWND hWnd, int width, int height)
//{
//    // スワップチェインの設定
//    DXGI_SWAP_CHAIN_DESC scDesc = {};
//    scDesc.BufferCount = 1;
//    scDesc.BufferDesc.Width = width;
//    scDesc.BufferDesc.Height = height;
//    scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//    scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//    scDesc.OutputWindow = hWnd;
//    scDesc.SampleDesc.Count = 1;
//    scDesc.Windowed = TRUE;
//
//    // デバイス & スワップチェイン作成
//    HRESULT hr = D3D11CreateDeviceAndSwapChain(
//        nullptr,
//        D3D_DRIVER_TYPE_HARDWARE,
//        nullptr,
//        0,
//        nullptr, 0,
//        D3D11_SDK_VERSION,
//        &scDesc,
//        &g_SwapChain,
//        &g_Device,
//        nullptr,
//        &g_Context
//    );
//
//    if (FAILED(hr)) return false;
//
//    // バックバッファ取得 → RTV作成
//    ID3D11Texture2D* backBuffer = nullptr;
//    g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
//    g_Device->CreateRenderTargetView(backBuffer, nullptr, &g_RTV);
//    backBuffer->Release();
//
//    return true;
//}
//
////DirectXの描画開始
//void BeginDraw(float r, float g, float b, float a)
//{
//    float clearColor[4] = { r, g, b, a };
//    g_Context->OMSetRenderTargets(1, &g_RTV, nullptr);
//    g_Context->ClearRenderTargetView(g_RTV, clearColor);
//}
//
////描画終了
//void EndDraw()
//{
//    g_SwapChain->Present(1, 0);
//}
//
//
