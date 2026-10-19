#pragma once
#include <d3d12.h>
#include "SceneEnums.h"

class RenderBlackboard;

struct RenderContext {
    ID3D12GraphicsCommandList* cmdList = nullptr;
    Scenes::ID sceneID = Scenes::ID::NONE; 
    float deltaTime = 0.0f;

    // 描画先・参照用デスクリプタ
    D3D12_CPU_DESCRIPTOR_HANDLE backBufferRTV{};
    D3D12_CPU_DESCRIPTOR_HANDLE mainDSV{};
    UINT screenWidth = 0;
    UINT screenHeight = 0;

    // パス間リソース共有用ブラックボード
    RenderBlackboard* blackboard = nullptr;
};
