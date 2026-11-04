#pragma once
#include "IRenderPass.h"
#include "GBuffer.h"
#include "ObjectTag.h"
#include <cstdint>
#include <memory>
#include <string>

class GBufferPass : public IRenderPass {
public:
    enum class ObjectFilterPreset {
        All = 0,
        GroundAndBackground = 1,
        Characters = 2,
        UIOnly = 3,
        Custom = 4
    };

    GBufferPass();
    virtual ~GBufferPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    int GetLayerFilter() const { return m_layerFilter; }
    void SetLayerFilter(int filter) { m_layerFilter = filter; }

    ObjectFilterPreset GetFilterPreset() const { return m_filterPreset; }
    void SetFilterPreset(ObjectFilterPreset preset);

    uint32_t GetTagMask() const { return m_tagMask; }
    void SetTagMask(uint32_t mask) { m_tagMask = mask; m_filterPreset = ObjectFilterPreset::Custom; }

    bool IsClearDepthEnabled() const { return m_clearDepthEnabled; }
    void SetClearDepthEnabled(bool clear) { m_clearDepthEnabled = clear; }

    GBuffer* GetGBuffer() { return m_gBuffer.get(); }

private:
    std::unique_ptr<GBuffer> m_gBuffer;
    int m_layerFilter = 0; // 0: Opaque by default for GBuffer
    ObjectFilterPreset m_filterPreset = ObjectFilterPreset::All;
    uint32_t m_tagMask = 0xFFFFFFFFu;
    bool m_clearDepthEnabled = true;
    std::string m_customName;
};
