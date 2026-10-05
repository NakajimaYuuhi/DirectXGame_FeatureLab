#pragma once
#include "IRenderPass.h"

class ShadowMapPass : public IRenderPass
{
public:
    ShadowMapPass() = default;
    virtual ~ShadowMapPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "Shadow Map Pass"; }
};
