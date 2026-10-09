#include "RenderPipeline.h"

void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass)
{
    m_passes.push_back(std::move(pass));
}

void RenderPipeline::Init(ID3D12Device* pDevice)
{
    for (auto& pass : m_passes) {
        if (pass) {
            pass->Init(pDevice);
        }
    }
}

void RenderPipeline::Execute(const RenderContext& ctx)
{
    for (auto& pass : m_passes) {
        if (!pass || !pass->IsEnabled()) {
            continue;
        }

        pass->Execute(ctx);
    }
}

void RenderPipeline::ClearPasses()
{
    m_passes.clear();
}
