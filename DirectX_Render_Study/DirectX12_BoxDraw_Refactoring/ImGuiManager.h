#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <Windows.h>
#include <wrl/client.h>

#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "imgui.h"
#include "DescriptorHeapAllocator.h"

class CImGuiManager {
public:

    //いらない気がする
    struct SrvAllocUserData
    {
        CImGuiManager* self;
    };

    static CImGuiManager& GetInstance();

    //画面の拡大率の取得
    float GetSystemScaleFactor();

    //実際に拡大する倍率の取得
    //拡大率にしたがって拡大するのが理想だが、今使ってるノートパソコンだと大きくて困るので、適用するかをフラグで決める
    float GetActualScaleFactor();


    // 初期化 (ウィンドウハンドルとDX12のデバイス、バッファ数、フォーマットが必要)
    bool Initialize(HWND hwnd);

    // 終了処理
    void Finalize();

    // フレーム開始時の処理
    void Begin();

    // フレーム終了時の描画処理 (コマンドリストを渡す)
    void End(ID3D12GraphicsCommandList* commandList);

    CDescriptorHeapAllocator& GetDescriptorHeapAllocator() { return m_DescriptorHeapAllocator; }

private:
    CImGuiManager() = default;
    ~CImGuiManager() = default;

    // ImGui用のフォントテクスチャ等を配置するためのSRVヒープ
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_srvHeap;
    CDescriptorHeapAllocator m_DescriptorHeapAllocator;
    SrvAllocUserData m_SrvAllocUserData;

};