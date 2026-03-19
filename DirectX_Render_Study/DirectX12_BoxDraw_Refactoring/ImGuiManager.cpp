#include "ImGuiManager.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

CImGuiManager::CImGuiManager()
    : m_hWnd(nullptr)
    , m_pDevice(nullptr)
    , m_pSrvHeap(nullptr)
    , m_FrameCount(0)
{
}

CImGuiManager::~CImGuiManager()
{
    Finalize();
}

bool CImGuiManager::Initialize(
    HWND hwnd,
    ID3D12Device* pDevice,
    ID3D12DescriptorHeap* pSrvHeap,
    int frameCount
)
{
    m_hWnd = hwnd;
    m_pDevice = pDevice;
    m_pSrvHeap = pSrvHeap;
    m_FrameCount = frameCount;

    // ImGui Core
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    // PlatformiWin32j
    if (!ImGui_ImplWin32_Init(hwnd))
        return false;

    // DX12 backend
    if (!ImGui_ImplDX12_Init(
        pDevice,
        frameCount,
        DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        pSrvHeap,
        pSrvHeap->GetCPUDescriptorHandleForHeapStart(),
        pSrvHeap->GetGPUDescriptorHandleForHeapStart()
    ))
        return false;

    m_IsInitialized = true;
    return true;
}

void CImGuiManager::BeginFrame()
{
    if (!m_IsInitialized) return;

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX12_NewFrame();
    ImGui::NewFrame();
}

void CImGuiManager::EndFrame()
{
    if (!m_IsInitialized) return;

    ImGui::EndFrame();
}

void CImGuiManager::Render(ID3D12GraphicsCommandList* pCommandList)
{
    if (!m_IsInitialized) return;

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), pCommandList);
}

void CImGuiManager::Finalize()
{
    if (!m_IsInitialized) return;

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_IsInitialized = false;
}