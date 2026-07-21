#pragma once
#include <vector>
#include <memory>
#include "IRenderPass.h"
#include "RenderContext.h"

class RenderPipeline {
public:
    RenderPipeline() = default;
    ~RenderPipeline() = default;

    // パスをパイプラインの末尾に追加する
    void AddPass(std::unique_ptr<IRenderPass> pass);

    // 登録されたすべてのパスを初期化する
    void Init(ID3D12Device* pDevice);

    // 毎フレーム呼ばれ、登録された順にパスを実行する
    void Execute(const RenderContext& ctx);

    // 必要に応じてパスをクリアする処理
    void ClearPasses();

private:
    std::vector<std::unique_ptr<IRenderPass>> m_passes;
};