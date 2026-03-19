#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>
#include <wrl/client.h>

#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "imgui.h"

class CImGuiManager {
public:
    static CImGuiManager& GetInstance();

    // 初期化 (ウィンドウハンドルとDX12のデバイス、バッファ数、フォーマットが必要)
    bool Initialize(HWND hwnd);

    // 終了処理
    void Finalize();

    // フレーム開始時の処理
    void Begin();

    // フレーム終了時の描画処理 (コマンドリストを渡す)
    void End(ID3D12GraphicsCommandList* commandList);

private:
    CImGuiManager() = default;
    ~CImGuiManager() = default;

    // ImGui用のフォントテクスチャ等を配置するためのSRVヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
};