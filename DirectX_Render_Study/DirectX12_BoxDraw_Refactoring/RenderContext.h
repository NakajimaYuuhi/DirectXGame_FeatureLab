#pragma once
#include <d3d12.h>
// #include "Scene.h" // Scenes::ID を使用する場合
// #include "Camera.h" // Cameraクラスを参照する場合

struct RenderContext {
    ID3D12GraphicsCommandList* cmdList;
    int sceneID; // ※実際の Scenes::ID の型に合わせてください
    float deltaTime;

    // 描画先・参照元となるリソースのハンドル
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV;
    D3D12_CPU_DESCRIPTOR_HANDLE mainDSV;
    UINT screenWidth;
    UINT screenHeight;

    // 将来のポストプロセス・コンピュートパス等で参照するためのカメラ情報
    // (値のバインドはせず、必要なパスが View/Proj 行列を取得するためだけに使用)
    const Camera* pCamera = nullptr; 
};