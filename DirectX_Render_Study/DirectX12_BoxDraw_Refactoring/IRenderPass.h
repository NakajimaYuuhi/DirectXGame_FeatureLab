#pragma once
#include <d3d12.h>
#include <string>
#include "RenderContext.h"

class IRenderPass {
public:
    virtual ~IRenderPass() = default;

    // パスの初期化（PSOの生成や必要なリソースの確保など）
    virtual void Init(ID3D12Device* pDevice) = 0;

    // コマンドリストへのコマンド積み込み処理
    virtual void Execute(const RenderContext& ctx) = 0;

    // デバッグやプロファイリング用にパスの名前を返せるようにしておくと便利です
    virtual std::string GetName() const = 0;
};