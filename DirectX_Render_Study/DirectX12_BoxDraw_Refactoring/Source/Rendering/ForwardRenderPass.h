#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h" // 霑ｽ蜉

class ForwardRenderPass : public IRenderPass {
public:
    // コンストラクタで出力先のテクスチャを受け取る (nullptrなら画面へ直接出力)
    ForwardRenderPass(RenderTexture* pDestTex = nullptr);
    virtual ~ForwardRenderPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

private:
    // 出力先のテクスチャを保存しておくメンバ変数
    RenderTexture* m_pDestTex = nullptr;
};