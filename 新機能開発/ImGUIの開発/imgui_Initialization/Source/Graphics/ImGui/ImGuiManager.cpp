#include "ImGuiManager.h"

// ImGui
#include "../Source/External/imgui/imgui.h"
#include "../Source/External/imgui/imgui_impl_win32.h"
#include "../Source/External/imgui/imgui_impl_dx11.h"



CImGuiManager::CImGuiManager() {}
CImGuiManager::~CImGuiManager()
{
    Shutdown();
}

bool CImGuiManager::Init(HWND hWnd, ID3D11Device* device, ID3D11DeviceContext* context)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX11_Init(device, context);

    return true;
}

void CImGuiManager::Begin()
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void CImGuiManager::End()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void CImGuiManager::Shutdown()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}
