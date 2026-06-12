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
//  2026/03/12 リファクタリングは一旦完了
// 
//  共通の頂点データを使っているときなどは、共通のバッファを使う
//  PSO管理
//                                       
///////////////////////////////////////////

//===== インクルード =====
#include <windows.h>
#include "PSOManager.h"
#include "DX12Manager.h"
#include "InputManager.h"	//TODO:mainが知ってる必要は無い気がする

//Scene
#include "Scene.h"
#include "SceneTest.h"

//スマートポインタ
#include<memory>
template<typename T>
using UniquePtr = std::unique_ptr<T>;

#include "ImGuiManager.h"

#include "imgui_impl_win32.h"

#include <string>

#include "gltfLoader.h"

//===== 名前空間宣言 =====

//===== 定数・マクロ定義 =====

//===== 構造体定義 =====

//===== グローバル変数宣言 =====
UniquePtr<CScene> g_CScene;

//===== プロトタイプ宣言 =====

// ImGuiのWin32実装にあるプロトタイプ宣言
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool initialized = false;
void DisplaySize()
{
	if(!initialized)return;

	ImGuiIO& io = ImGui::GetIO();
	std::string str = "DisplaySize = ";
	str += io.DisplaySize.x;
	str += ", ";
	str += io.DisplaySize.y;
	OutputDebugString(str.c_str());
	OutputDebugString("\n");
}

//===== 関数定義 =====
//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	// ImGuiがメッセージを処理したらそこでリターン
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;


	switch (msg)
	{
	case WM_SIZE:
		
		//DisplaySize();

		if (DX12Manager::GetInstance().GetDevice() && wparam != SIZE_MINIMIZED)
		{
			DX12Manager::GetInstance().ResizeRenderTarget(lparam);
			DX12Manager::GetInstance().ResizeDepthBuffer(lparam);
			DX12Manager::GetInstance().ResizeViewPort(lparam);
		}
		//DisplaySize();
		return 0;
	case WM_SYSCOMMAND:
		if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Todo : Windowもクラス化したい
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

	//ImGuiの機能でDPIを取得
	float main_scale = CImGuiManager::GetInstance().GetActualScaleFactor();
	//main_scale /= 1.5;
	RECT rc = { 0, 0, 1920, 1080 }; // ほしい「中身」のサイズ
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE); // 枠を含めたサイズに計算し直してくれる

	// rc.right - rc.left が「枠を含めた本当の幅」になる
	HWND hwnd = CreateWindowEx(
		0, className, "DirectX12 Window", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		(rc.right - rc.left) * main_scale, (rc.bottom - rc.top) * main_scale, // 計算したサイズを渡す！
		nullptr, nullptr, hInst, nullptr
	);


	////ウィンドウの作成
	//HWND hwnd = CreateWindowEx(
	//	0,
	//	className,
	//	"DirectX12 Window",
	//	WS_OVERLAPPEDWINDOW,
	//	100, 100, 1920 * main_scale, 1080 * main_scale,
	//	NULL,
	//	NULL,
	//	hInst,
	//	NULL
	//);

	//ウィンドウの表示
	ShowWindow(hwnd, nCmdShow);

	MSG msg = {};


	//DirectX12の初期化
	DX12Manager::GetInstance().Initialize(hwnd);
	PSOManager::GetInstance().Init(DX12Manager::GetInstance().GetDevice());
	
	//最初のシーンの作成
	g_CScene = std::make_unique<CSceneTest>();

	//シーンの初期化
	g_CScene->Init();

	CInputManager::GetInstance();

	CImGuiManager::GetInstance().Initialize(hwnd);
	initialized = true;

	//DisplaySize();

	//TestLoadGLTF();

	bool done = false;
	while (!done)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;


		//




		//画面が隠れているならフレームをスキップ
		if (DX12Manager::GetInstance().IsOccluded(hwnd))
		{
			::Sleep(10);
			continue;
		}


		//---入力の更新---
		CInputManager::GetInstance().Update();

			

		// --- 更新 ---
		//ImGuiのフレーム開始

		CImGuiManager::GetInstance().Begin();

		{
			bool IsValidTransform;
			//ImGuiの描画命令を溜める
			ImGui::Begin("ComponentList"); // ここでGUIを作る
			ImGui::Text("Transform"); ImGui::SameLine(); ImGui::Checkbox(" ", &IsValidTransform);
			ImGui::Text("Position");
			ImGui::Text("Scale");
			ImGui::Text("Rotation");
			ImGui::End();

		}
		{
			static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

			static bool show_demo_window = true;
			static bool show_another_window = true;

			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			//クリックされたらTrueが返ってくる
			if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();//同じ行に各命令？
			ImGui::Text("counter = %d", counter);//テキストは、Printfみたいに書ける ってかC++がそういうもんなのか？

			//FrameRateはioから持ってこれる
			//ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
			ImGui::End();
		}

		//シーンの更新処理
		g_CScene->Update();


		//---描画処理---
		//DirectX12描画開始
		DX12Manager::GetInstance().BeginDraw();

		//シーンの描画
		g_CScene->Draw();

		//ImGuiの描画
		CImGuiManager::GetInstance().End(DX12Manager::GetInstance().GetCommandList());

		//DirectX12の描画終了
		DX12Manager::GetInstance().EndDraw();
		
	}

	DX12Manager::GetInstance().Finalize();

	return 0;
}




