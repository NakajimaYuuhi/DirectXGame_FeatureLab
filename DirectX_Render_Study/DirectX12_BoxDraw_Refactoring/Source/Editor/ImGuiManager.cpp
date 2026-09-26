#include "ImGuiManager.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "DX12Manager.h"
#include "BasicSettings.h"

CImGuiManager& CImGuiManager::GetInstance() {
    static CImGuiManager instance;
    return instance;
}

//????g????ï
float CImGuiManager::GetSystemScaleFactor()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    return main_scale;
}



float CImGuiManager::GetActualScaleFactor()
{
    float scale = GetSystemScaleFactor();

    //false???K?p?????
    if (!DISPLAY_SCALING_ENABLED)return 1.0f;

    //true???g???????????
    return GetSystemScaleFactor();
}

bool CImGuiManager::Initialize(HWND hwnd)
{

    float main_scale = GetActualScaleFactor();


    auto& dx12 = DX12Manager::GetInstance();
    ID3D12Device* device = dx12.GetDevice(); // CDX12Manager??GetDevice()???K?v
    ID3D12CommandQueue* commandQueue = dx12.GetCommandQueue();

    // ???S?`?F?b?N?F?f?o?C?X?????????G???[?I
    if (!device) {
        OutputDebugString("Error: DX12 Device is NULL!\n");
        return false;
    }


    // 1. ImGui?R???e?L?X?g???
    IMGUI_CHECKVERSION();           //?o?[?W?????`?F?b?N
    ImGui::CreateContext();         //?R???e?L?X?g(?O???[?o??????)??
    ImGuiIO& io = ImGui::GetIO();   //???o?????????iImGuiIO?j??G??????ï
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msgothic.ttc", 20.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //io.Fonts->Build();

    // ?X?^?C?????
    ImGui::StyleColorsDark();

    // 2. ImGui?p??SRV?f?B?X?N???v?^?q?[?v??
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 3;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap)))) {
        return false;
    }

    //?f?B?X?N???v?^?q?[?v??A???P?[?^?????????
    m_DescriptorHeapAllocator.Create(device, m_srvHeap.Get());

    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;




    // 3. ?v???b?g?t?H?[????????_???[???????
    //ImGui_ImplWin32_Init(hwnd);




    if (!ImGui_ImplWin32_Init(hwnd)) return false;




    //DirectX??A???????
    //?????????????????????????Init??????AInitInfo?????????

    ImGui_ImplDX12_InitInfo init_info = {};
    init_info.Device = device;            //?f?o?C?X
    init_info.CommandQueue = commandQueue;//?R?}???h?L???[??K?v??????
    init_info.NumFramesInFlight = FRAME_BUFFER_COUNT;//?t???[???o?b?t?@??? ?X???b?v?`?F?[????o?b?N?o?b?t?@??????????(???2??3)
    init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;//RTVFormat
    init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;  //?[?x?X?e???V????t?H?[?}?b?g?i?g??????? UNKNOWN ??OK)
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

    //----- ?t?H???g??Z?b?g -----
    ImFontConfig config;
    config.SizePixels = 18.0f;

    // ???{???????Z?b?g????
    static const ImWchar japanese_range[] = {
        0x0020, 0x00FF,   // Basic Latin
        0x3000, 0x30FF,   // ?????E?J?^?J?i
        0x4E00, 0x9FAF,   // ?????i??{?j
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

    // ImGui?????f?[?^??????_?????O?p??????
    //??????`?Ž{???`??f?[?^(ImDrawData)????????
    ImGui::Render();

    // ?`???q?[?v??ImGui?p???????
    ID3D12DescriptorHeap* heaps[] = { m_srvHeap.Get() };
    commandList->SetDescriptorHeaps(_countof(heaps), heaps);

    // ?R?}???h???X?g??ImGui??`??R?}???h????(GPU?????)
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
}

void CImGuiManager::Finalize() {
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    m_srvHeap.Reset();
}