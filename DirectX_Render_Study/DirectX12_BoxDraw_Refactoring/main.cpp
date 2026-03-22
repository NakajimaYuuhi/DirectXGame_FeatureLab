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

//===== 名前空間宣言 =====

//===== 定数・マクロ定義 =====

//===== 構造体定義 =====

//===== グローバル変数宣言 =====
UniquePtr<CScene> g_CScene;

//===== プロトタイプ宣言 =====

// ImGuiのWin32実装にあるプロトタイプ宣言
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


//===== 関数定義 =====
//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{

	// ImGuiがメッセージを処理したらそこでリターン
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
		return true;

	switch (msg)
	{
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


	//DirectX12の初期化
	CDX12Manager::GetInstance().Initialize(hwnd);
	
	//最初のシーンの作成
	g_CScene = std::make_unique<CSceneTest>();

	//シーンの初期化
	g_CScene->Init();

	CInputManager::GetInstance();

	CImGuiManager::GetInstance().Initialize(hwnd);

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//画面が隠れているならフレームをスキップ
			if (CDX12Manager::GetInstance().IsOccluded(hwnd))
			{
				::Sleep(10);
				continue;
			}


			//---入力の更新---
			CInputManager::GetInstance().Update();

			

			// --- 更新 ---
			//ImGuiのフレーム開始
			CImGuiManager::GetInstance().Begin();

			//ImGuiの描画命令を溜める
			ImGui::Begin("Debug Window"); // ここでGUIを作る
			ImGui::Text("Hello, DX12!");
			ImGui::End();

			//シーンの更新処理
			g_CScene->Update();


			//---描画処理---
			//DirectX12描画開始
			CDX12Manager::GetInstance().BeginDraw();

			//シーンの描画
			g_CScene->Draw();

			//ImGuiの描画
			CImGuiManager::GetInstance().End(CDX12Manager::GetInstance().GetCommandLIst());

			//DirectX12の描画終了
			CDX12Manager::GetInstance().EndDraw();
		}
	}

	CDX12Manager::GetInstance().Finalize();

	return 0;
}

