///////////////////////////////////////////
//main.cpp                               
//                                       
//  imguiの初期化プログラム               
//                                         
//  ～概要～                             
//    imguiを初期化する
//                                       
//  ～更新履歴～
// 
//  2025/01/01 -Nakajima Yuhi-
//  制作開始
// 
//  2025/01/01 -Nakajima Yuhi-
//  DirectXの最低限の初期化
// 
//  2025/01/01 -Nakajima Yuhi-
//  imguiの初期化開始
//                                       
///////////////////////////////////////////

#include "Window/Window.h"
#include "DirectX11/DirectX11.h"
#include "../Graphics/ImGui//ImGuiManager.h"//これじゃないといけないとかは無いはず
#include "../External/imgui/imgui.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Window window;
    DirectX11 dx;
    CImGuiManager imgui;


    const int WIDTH = 1280;
    const int HEIGHT = 720;

    if (!window.Create("DX11 + ImGui", WIDTH, HEIGHT))
        return -1;

    if (!dx.Init(window.GetHWND(), WIDTH, HEIGHT))
        return -1;

    while (window.ProcessMessage())
    {
        dx.BeginFrame(0.1f, 0.2f, 0.3f, 1.0f);

        // --- ここに今後 ImGui や描画コードを書く ---
        imgui.Begin();

        ImGui::Begin("Hello");
        ImGui::Text("ImGui is separate now!");
        ImGui::End();

        imgui.End();


        dx.EndFrame();
    }

    return 0;
}