#include "RenderPipeline.h"

void RenderPipeline::AddPass(std::unique_ptr<IRenderPass> pass, ID3D12Device* pDevice)
{
    if (!pass) return;

    if (pDevice) {
        pass->Init(pDevice);
    }
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
    RenderContext localCtx = ctx;
    if (!localCtx.blackboard) {
        localCtx.blackboard = &m_blackboard;
    }

    for (auto& pass : m_passes) {
        if (!pass || !pass->IsEnabled()) {
            continue;
        }

        pass->Execute(localCtx);
    }
}

bool RenderPipeline::MovePass(size_t fromIndex, size_t toIndex)
{
    if (fromIndex >= m_passes.size() || toIndex >= m_passes.size() || fromIndex == toIndex) {
        return false;
    }

    auto pass = std::move(m_passes[fromIndex]);
    m_passes.erase(m_passes.begin() + fromIndex);
    m_passes.insert(m_passes.begin() + toIndex, std::move(pass));
    return true;
}

bool RenderPipeline::RemovePass(size_t index)
{
    if (index >= m_passes.size()) {
        return false;
    }

    m_passes.erase(m_passes.begin() + index);
    return true;
}

void RenderPipeline::ClearPasses()
{
    m_passes.clear();
}
