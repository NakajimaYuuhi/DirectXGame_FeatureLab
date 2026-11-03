#pragma once
#include "IRenderPass.h"
#include <string>

class ShadowMapPass : public IRenderPass
{
public:
    ShadowMapPass() = default;
    virtual ~ShadowMapPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return "Shadow Map Pass"; }

    // Custom shadow shader & rasterizer depth bias settings
    void SetShaderPath(const std::string& path) { m_shaderPath = path; }
    const std::string& GetShaderPath() const { return m_shaderPath; }

    void SetVsEntry(const std::string& entry) { m_vsEntry = entry; }
    const std::string& GetVsEntry() const { return m_vsEntry; }

    void SetDepthBias(int bias) { m_depthBias = bias; }
    int GetDepthBias() const { return m_depthBias; }

    void SetSlopeScaledDepthBias(float slopeBias) { m_slopeScaledDepthBias = slopeBias; }
    float GetSlopeScaledDepthBias() const { return m_slopeScaledDepthBias; }

private:
    std::string m_shaderPath = "Assets/Shader/ShadowMap.hlsl";
    std::string m_vsEntry = "VSMain";
    int m_depthBias = 25;
    float m_slopeScaledDepthBias = 0.35f;
};
