#include "Window.h"
#include "../Source/External/imgui/imgui.h"
#include "../Source/External/imgui/imgui_impl_win32.h"


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Window::Window()
{
    m_hInstance = GetModuleHandle(nullptr);
}

Window::~Window()
{
    if (m_hWnd)
        DestroyWindow(m_hWnd);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
        return true;

    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wp, lp);
}

bool Window::Create(const char* title, int width, int height)
{
    // クラス名をマルチバイトで定義
    const char* className = "MyWindowClass";

    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = m_hInstance;
    wc.lpszClassName = className;

    RegisterClassA(&wc);

    RECT rect = { 0, 0, width, height };
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

    m_hWnd = CreateWindowA(
        className,
        title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr,
        m_hInstance, nullptr
    );

    if (!m_hWnd) return false;

    ShowWindow(m_hWnd, SW_SHOW);

    return true;
}

bool Window::ProcessMessage()
{
    MSG msg = {};
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return true;
}
