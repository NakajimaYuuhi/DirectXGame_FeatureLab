#pragma once
#include "IRenderPass.h"

// もし ForwardRenderPass の中で ObjectManager を直接呼ぶ場合、
// 前方宣言するか、ここでインクルードします。
// class ObjectManager; 

class ForwardRenderPass : public IRenderPass {
public:
    // コンストラクタ
    ForwardRenderPass();
    virtual ~ForwardRenderPass() = default;

    // IRenderPass のオーバーライド
    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

private:
    // 将来的にオフスクリーン用の RenderTexture や ObjectManager のポインタなどを
    // メンバとして持たせる場合はここに追加します。
};