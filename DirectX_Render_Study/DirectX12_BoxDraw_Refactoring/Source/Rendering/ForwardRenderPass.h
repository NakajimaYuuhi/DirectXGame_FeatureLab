#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include "ObjectTag.h"
#include <cstdint>
#include <string>

class ForwardRenderPass : public IRenderPass {
public:
    enum class ObjectFilterPreset {
        All = 0,
        GroundAndBackground = 1, // BACKGROUND, FIELD
        Characters = 2,          // PLAYER, ENEMY, PLAYER_BULLET, ENEMY_BULLET
        UIOnly = 3,              // UI, TEXT
        Custom = 4
    };

    ForwardRenderPass(RenderTexture* pDestTex = nullptr);
    virtual ~ForwardRenderPass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override;

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    // Layer Filter (-1: All Layers, 0: Opaque, 1: Transparent, 2: Debug, 3: UI)
    int GetLayerFilter() const { return m_layerFilter; }
    void SetLayerFilter(int filter) { m_layerFilter = filter; }

    // Object Tag Filter Preset & Bitmask
    ObjectFilterPreset GetFilterPreset() const { return m_filterPreset; }
    void SetFilterPreset(ObjectFilterPreset preset);
    ObjectFilterPreset GetObjectFilterPreset() const { return m_filterPreset; }
    void SetObjectFilterPreset(ObjectFilterPreset preset) { SetFilterPreset(preset); }

    uint32_t GetTagMask() const { return m_tagMask; }
    void SetTagMask(uint32_t mask) { m_tagMask = mask; m_filterPreset = ObjectFilterPreset::Custom; }

    // Clear render target / depth stencil controls
    bool IsClearEnabled() const { return m_clearColorEnabled; }
    void SetClearEnabled(bool clear) { m_clearColorEnabled = clear; m_clearDepthEnabled = clear; }

    bool IsClearColorEnabled() const { return m_clearColorEnabled; }
    void SetClearColorEnabled(bool clear) { m_clearColorEnabled = clear; }

    bool IsClearDepthEnabled() const { return m_clearDepthEnabled; }
    void SetClearDepthEnabled(bool clear) { m_clearDepthEnabled = clear; }

private:
    RenderTexture* m_pDestTex = nullptr;
    int m_layerFilter = -1;
    ObjectFilterPreset m_filterPreset = ObjectFilterPreset::All;
    uint32_t m_tagMask = 0xFFFFFFFFu;
    bool m_clearColorEnabled = true;
    bool m_clearDepthEnabled = true;
    std::string m_customName;
};
