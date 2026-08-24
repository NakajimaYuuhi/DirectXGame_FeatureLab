#include "../UI/InspectorUI.h"
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



//スマートポインタ
#include<memory>
template<typename T>
using UniquePtr = std::unique_ptr<T>;

#include "ImGuiManager.h"

#include "imgui_impl_win32.h"

#include <string>

#include "SceneManager.h"

//音
#include <xaudio2.h>
#include <wrl/client.h> // Microsoft::WRL::ComPtr を使うと管理が楽です

#include "audio.h"

//===== 名前空間宣言 =====

//===== 定数・マクロ定義 =====

//===== 構造体定義 =====

//===== グローバル変数宣言 =====

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
	
	//音
	Audio::InitMaster();


	//----- SceneManagerの開始 -----
	SceneManager::GetInstance();

	CInputManager::GetInstance();

	CImGuiManager::GetInstance().Initialize(hwnd);
	initialized = true;

	//DisplaySize();


	//Microsoft::WRL::ComPtr<IXAudio2> pXAudio2;
	//IXAudio2MasteringVoice* pMasterVoice = nullptr; // ※COMではないので生ポインタか専用の管理が必要です

	//// COMの初期化（すでに行っている場合は不要）
	//CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//// XAudio2 エンジンの作成
	//HRESULT hr = XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//if (FAILED(hr)) { /* エラー処理 */ }

	//// マスタリングボイス（最終出力先）の作成
	//hr = pXAudio2->CreateMasteringVoice(&pMasterVoice);
	//if (FAILED(hr)) { /* エラー処理 */ }



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
			CInspectorUI::GetInstance().Draw();
		}

		//シーンの更新処理
		//g_CScene->Update();
		SceneManager::GetInstance().Update();


		//---描画処理---
		//DirectX12描画開始
		DX12Manager::GetInstance().BeginDraw();

		//シーンの描画
		SceneManager::GetInstance().Draw();

		//ImGuiの描画
		CImGuiManager::GetInstance().End(DX12Manager::GetInstance().GetCommandList());

		//DirectX12の描画終了
		DX12Manager::GetInstance().EndDraw();
		
	}

	SceneManager::GetInstance().Uninit();

	//Audioはオブジェクトを破棄した後に、終了させないといけない
	Audio::UninitMaster();
	DX12Manager::GetInstance().Finalize();

	return 0;
}




