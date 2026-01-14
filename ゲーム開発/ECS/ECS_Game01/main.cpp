// ===========================================================
// DirectX11 + ECS 最小サンプル
// 四角が画面中央に出て右にスーッと動く
// シェーダーもすべて main.cpp 内に直書き
// ===========================================================

#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <vector>
#include <DirectXMath.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// ===== DirectX グローバル =====
ID3D11Device* gDevice = nullptr;
ID3D11DeviceContext* gContext = nullptr;
IDXGISwapChain* gSwapChain = nullptr;
ID3D11RenderTargetView* gRTV = nullptr;

ID3D11VertexShader* gVS = nullptr;
ID3D11PixelShader* gPS = nullptr;
ID3D11InputLayout* gLayout = nullptr;
ID3D11Buffer* gVBO = nullptr;

// ===== ECS =====
using Entity = int;

struct Transform {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

struct Sprite {
    float size;
};

std::vector<Transform> transforms;
std::vector<Velocity>  velocities;
std::vector<Sprite>    sprites;

Entity CreateEntity(Transform t, Velocity v, Sprite s) {
    transforms.push_back(t);
    velocities.push_back(v);
    sprites.push_back(s);
    return (int)transforms.size() - 1;
}

// ===== Movement System =====
void UpdateMovement(float dt) {
    for (size_t i = 0; i < transforms.size(); i++) {
        transforms[i].x += velocities[i].vx * dt;
        transforms[i].y += velocities[i].vy * dt;
    }
}

// ===========================================================
// 四角の描画（毎フレーム Map で更新）
// ===========================================================
void RenderSprite(const Transform& t, float size, float screenW, float screenH)
{
    struct Vertex { float x, y; };

    float half = size * 0.5f;

    // ==== ピクセル座標を NDC (-1?1) に変換 ====
    auto toNDC = [&](float px, float py) {
        float x = (px / screenW) * 2.0f - 1.0f;
        float y = -(py / screenH) * 2.0f + 1.0f;
        return DirectX::XMFLOAT2(x, y);
        };

    // 四角の4頂点
    DirectX::XMFLOAT2 p0 = toNDC(t.x - half, t.y - half);
    DirectX::XMFLOAT2 p1 = toNDC(t.x + half, t.y - half);
    DirectX::XMFLOAT2 p2 = toNDC(t.x - half, t.y + half);
    DirectX::XMFLOAT2 p3 = toNDC(t.x + half, t.y + half);

    Vertex vtx[4] = {
        { p0.x, p0.y },
        { p1.x, p1.y },
        { p2.x, p2.y },
        { p3.x, p3.y },
    };

    // ===== VBO更新 =====
    D3D11_MAPPED_SUBRESOURCE ms{};
    gContext->Map(gVBO, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms);
    memcpy(ms.pData, vtx, sizeof(vtx));
    gContext->Unmap(gVBO, 0);

    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    gContext->IASetVertexBuffers(0, 1, &gVBO, &stride, &offset);
    gContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    gContext->Draw(4, 0);
}

// ===========================================================
// DirectX 初期化
// ===========================================================
HRESULT InitDirectX(HWND hWnd, int width, int height)
{
    DXGI_SWAP_CHAIN_DESC scd{};
    scd.BufferCount = 1;
    scd.BufferDesc.Width = width;
    scd.BufferDesc.Height = height;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hWnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed = TRUE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE,
        nullptr, 0,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &scd, &gSwapChain,
        &gDevice, nullptr,
        &gContext);

    if (FAILED(hr)) return hr;

    ID3D11Texture2D* backBuffer = nullptr;
    gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

    gDevice->CreateRenderTargetView(backBuffer, nullptr, &gRTV);
    backBuffer->Release();

    gContext->OMSetRenderTargets(1, &gRTV, nullptr);

    // ===== Viewport 設定 =====
    D3D11_VIEWPORT vp{};
    vp.Width = (float)width;
    vp.Height = (float)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    gContext->RSSetViewports(1, &vp);

    // ===== シェーダー（main.cpp 内に直書き） =====
    const char* vs_src =
        "struct VS_IN { float2 pos : POSITION; };"
        "struct PS_IN { float4 pos : SV_POSITION; };"
        "PS_IN VSMain(VS_IN input) {"
        "    PS_IN output;"
        "    output.pos = float4(input.pos, 0.0, 1.0);"
        "    return output;"
        "}";

    const char* ps_src =
        "float4 PSMain() : SV_TARGET {"
        "    return float4(1,1,1,1);"  // 白
        "}";

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errBlob = nullptr;

    D3DCompile(vs_src, strlen(vs_src), nullptr, nullptr, nullptr,
        "VSMain", "vs_4_0", 0, 0, &vsBlob, &errBlob);

    D3DCompile(ps_src, strlen(ps_src), nullptr, nullptr, nullptr,
        "PSMain", "ps_4_0", 0, 0, &psBlob, &errBlob);

    gDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &gVS);
    gDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &gPS);

    gContext->VSSetShader(gVS, nullptr, 0);
    gContext->PSSetShader(gPS, nullptr, 0);

    // ===== InputLayout =====
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    gDevice->CreateInputLayout(
        layoutDesc, 1,
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &gLayout);

    gContext->IASetInputLayout(gLayout);

    vsBlob->Release();
    psBlob->Release();

    // ===== VBO 作成（1回だけ） =====
    D3D11_BUFFER_DESC bd{};
    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth = sizeof(float) * 2 * 4; // 2D頂点 x 4
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    gDevice->CreateBuffer(&bd, nullptr, &gVBO);

    return S_OK;
}

// ===========================================================
// Window Proc
// ===========================================================
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(hWnd, msg, wp, lp);
}

// ===========================================================
// WinMain
// ===========================================================
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
    int width = 800;
    int height = 600;

    WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0,0,
        hInst, nullptr, nullptr, nullptr,
        nullptr, "ECS", nullptr };

    RegisterClassEx(&wc);

    HWND hWnd = CreateWindow("ECS", "DX11 ECS Sample",
        WS_OVERLAPPEDWINDOW, 100, 100, width, height,
        nullptr, nullptr, wc.hInstance, nullptr);

    ShowWindow(hWnd, SW_SHOWDEFAULT);

    InitDirectX(hWnd, width, height);

    // ==== Entity 作成（画面中央から右へ） ====
    CreateEntity(
        { 400, 300 },  // 画面中央 (ピクセル座標)
        { 80, 0 },     // 右へ80px/s
        { 100 }        // 大きさ100px
    );

    MSG msg{};
    DWORD prev = GetTickCount();

    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            DWORD cur = GetTickCount();
            float dt = (cur - prev) / 1000.0f;
            prev = cur;

            UpdateMovement(dt);

            float clear[4] = { 0.1f,0.1f,0.1f,1 };
            gContext->ClearRenderTargetView(gRTV, clear);

            // Render all entities
            for (size_t i = 0; i < transforms.size(); i++) {
                RenderSprite(transforms[i], sprites[i].size, width, height);
            }

            gSwapChain->Present(1, 0);
        }
    }

    return 0;
}
