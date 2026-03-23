#include "ImGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DX12Manager.h"

CImGuiManager& CImGuiManager::GetInstance() {
    static CImGuiManager instance;
    return instance;
}

bool CImGuiManager::Initialize(HWND hwnd) 
{

    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));


    auto& dx12 = CDX12Manager::GetInstance();
    ID3D12Device* device = dx12.GetDevice(); // CDX12ManagerにGetDevice()が必要
    ID3D12CommandQueue* commandQueue = dx12.GetCommandQueue();

    // 安全チェック：デバイスが空っぽならエラー！
    if (!device) {
        OutputDebugString("Error: DX12 Device is NULL!\n");
        return false;
    }


    // 1. ImGuiコンテキストの作成
    IMGUI_CHECKVERSION();           //バージョンチェック
    ImGui::CreateContext();         //コンテキスト(グローバルな状態)作成
    ImGuiIO& io = ImGui::GetIO();   //入出力に関する設定（ImGuiIO）を触れるように取得
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msgothic.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //io.Fonts->Build();

    // スタイル設定
    ImGui::StyleColorsDark();

    // 2. ImGui用のSRVディスクリプタヒープ作成
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 3;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)))) {
        return false;
    }

    //ディスクリプタヒープのアロケータも作っておく
    m_DescriptorHeapAllocator.Create(device, m_srvHeap.Get());

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;




    // 3. プラットフォームとレンダラーの初期化
    //ImGui_ImplWin32_Init(hwnd);




    if (!ImGui_ImplWin32_Init(hwnd)) return false;




    //DirectX関連の初期化
    //引数が多くて分かりにくかったInit処理を、InitInfoにまとめたもの

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = device;            //デバイス
    init_info.CommandQueue = commandQueue;//コマンドキューも必要になった
    init_info.NumFramesInFlight = FRAME_BUFFER_COUNT;//フレームバッファの数 スワップチェーンのバックバッファの数と合わせる(大体2か3)
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;//RTVFormat
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;  //深度ステンシルのフォーマット（使わないなら UNKNOWN でOK)
    init_info.SrvDescriptorHeap = m_srvHeap.Get();
    init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return CImGuiManager::GetInstance().GetDescriptorHeapAllocator().Alloc(out_cpu_handle, out_gpu_handle); };
    init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle) { return CImGuiManager::GetInstance().GetDescriptorHeapAllocator().Free(cpu_handle, gpu_handle); };
    

    //ImGui_ImplDX12_Init(
    //    device,
    //    FRAME_BUFFER_COUNT,
    //    DXGI_FORMAT_R8G8B8A8_UNORM,
    //    m_srvHeap.Get(),
    //    m_srvHeap->GetCPUDescriptorHandleForHeapStart(),
    //    m_srvHeap->GetGPUDescriptorHandleForHeapStart()
    //);

    //----- フォントのセット -----
    ImFontConfig config;
    config.SizePixels = 18.0f;

    // 日本語の文字セットを追加
    static const ImWchar japanese_range[] = {
        0x0020, 0x00FF,   // Basic Latin
        0x3000, 0x30FF,   // ひらがな・カタカナ
        0x4E00, 0x9FAF,   // 漢字（基本）
        0,
    };


    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\yumin.ttf", 20.0f, &config, io.Fonts->GetGlyphRangesJapanese());

    if (!ImGui_ImplDX12_Init(&init_info))
    {
        OutputDebugStringA("Error: ImGui_ImplDX12_Init Failed!\n");
        return false;
    }




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