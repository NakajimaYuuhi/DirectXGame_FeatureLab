#include "IMGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DX12Manager.h"

CImGuiManager& CImGuiManager::GetInstance() {
    static CImGuiManager instance;
    return instance;
}

bool CImGuiManager::Initialize(HWND hwnd) {
    auto& dx12 = CDX12Manager::GetInstance();
    ID3D12Device* device = dx12.GetDevice(); // CDX12ManagerにGetDevice()が必要

    // 安全チェック：デバイスが空っぽならエラー！
    if (!device) {
        OutputDebugStringA("Error: DX12 Device is NULL!\n");
        return false;
    }


    // 1. ImGuiコンテキストの作成
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msgothic.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    io.Fonts->Build();

    // スタイル設定（お好みで！）
    ImGui::StyleColorsDark();

    // 2. ImGui用のSRVディスクリプタヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 3;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)))) {
        return false;
    }

    // 3. プラットフォームとレンダラーの初期化
    //ImGui_ImplWin32_Init(hwnd);

    if (!ImGui_ImplWin32_Init(hwnd)) return false;


    //ImGui_ImplDX12_Init(
    //    device,
    //    FRAME_BUFFER_COUNT,
    //    DXGI_FORMAT_R8G8B8A8_UNORM,
    //    m_srvHeap.Get(),
    //    m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
    //    m_srvHeap->GetGPUDescriptorHandleForHeapStart()
    //);

    if (!ImGui_ImplDX12_Init(
        device,
        FRAME_BUFFER_COUNT,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        m_srvHeap.Get(),
        m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_srvHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        OutputDebugStringA("Error: ImGui_ImplDX12_Init Failed!\n");
        return false;
    }



    //ImGuiIO& io = ImGui::GetIO();
    //unsigned char* pixels;
    //int width, height;
    //io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    ImGui_ImplDX12_CreateDeviceObjects(); // GPU側に無理やり作る
    //ImGui::GetIO().Fonts->Build();        // CPU側で「ビルド済み」フラグを立てる

    return true;
}

void CImGuiManager::Begin() 
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void CImGuiManager::End(ID3D12GraphicsCommandList* commandList) 
{

    // ImGuiの内部データをレンダリング用にまとめる
    //貯めた描画命令を描画データ(ImDrawData)に変換する
    ImGui::Render();

    // 描画先ヒープをImGui用のものに設定
    ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);

    // コマンドリストにImGuiの描画コマンドを積む(GPUに送る)
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void CImGuiManager::Finalize() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_srvHeap.Reset();
}