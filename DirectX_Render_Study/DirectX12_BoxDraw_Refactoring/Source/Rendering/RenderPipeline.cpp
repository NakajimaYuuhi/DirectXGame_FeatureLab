#include "RenderPipeline.h"

void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
{
    m_passes.push_back(std::move(pass));
}

void RenderPipeline::Init(ID3D12Device* pDevice)
{
    for (auto& pass : m_passes) {
        pass->Init(pDevice);
    }
}

void RenderPipeline::Execute(const RenderContext& ctx)
{
    for (auto& pass : m_passes) {
        // オプション: ここで PIXBeginEvent 等を仕込むと、
        // Graphics Debugger でパスごとの処理時間が可視化されて非常に便利です。
        // PIXBeginEvent(ctx.cmdList, PIX_COLOR_DEFAULT, pass->GetName().c_str());

        pass->Execute(ctx);

        // PIXEndEvent(ctx.cmdList);
    }
}

void RenderPipeline::ClearPasses()
{
    m_passes.clear();
}