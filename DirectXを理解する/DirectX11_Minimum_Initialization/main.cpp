#include "Window.h"
#include "DirectX11.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    Window window;
    DirectX11 dx;

    const int WIDTH = 1280;
    const int HEIGHT = 720;

    if (!window.Create("DirectX11 Window", WIDTH, HEIGHT))
        return -1;

    if (!dx.Init(window.GetHWND(), WIDTH, HEIGHT))
        return -1;

    while (window.ProcessMessage())
    {
        dx.BeginFrame(0.1f, 0.2f, 0.3f, 1.0f);

        // --- Ç±Ç±Ç…ç°å„ ImGui Ç‚ï`âÊÉRÅ[ÉhÇèëÇ≠ ---

        dx.EndFrame();
    }

    return 0;
}
