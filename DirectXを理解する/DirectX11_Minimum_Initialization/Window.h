#pragma once
#include <Windows.h>

class Window
{
public:
    Window();
    ~Window();

    bool Create(const char* title, int width, int height);
    HWND GetHWND() const { return m_hWnd; }
    bool ProcessMessage();

private:
    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    HWND m_hWnd = nullptr;
    HINSTANCE m_hInstance = nullptr;
};
