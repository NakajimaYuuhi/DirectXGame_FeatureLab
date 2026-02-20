///////////////////////////////////////////
//main.cpp                               
//                                       
//  DirectX12の初期化プログラム               
//                                         
//  ～概要～                             
//    DirectX12の初期化を行うプログラム
//                                       
//  ～更新履歴～                         
//  2026/02/20 制作開始 -Nakajima Yuhi-  
//   
//                                       
///////////////////////////////////////////

//===== インクルード =====
#include <windows.h>
#include "DX12Manager.h"

//===== 名前空間宣言 =====

//===== 定数・マクロ定義 =====

//===== 構造体定義 =====

//===== グローバル変数宣言 =====

//===== プロトタイプ宣言 =====


//===== 関数定義 =====
//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

//エントリーポイント
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow)
{

	//ウィンドウクラスの登録
    const char* className = "DX12WindowClass";

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = className;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassEx(&wc);

	//ウィンドウの作成
    HWND hwnd = CreateWindowExA(
        0,
        className,
        "DirectX12 Window",
        WS_OVERLAPPEDWINDOW,
        100, 100, 1920, 1080,
        NULL,
        NULL,
        hInst,
        NULL
    );

	//ウィンドウの表示
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};

    CDX12Manager::GetInstance().Initialize(hwnd);

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

