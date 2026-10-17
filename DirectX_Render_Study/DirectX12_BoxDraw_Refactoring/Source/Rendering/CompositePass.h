#pragma once
#include "IRenderPass.h"
#include "RenderTexture.h"
#include <wrl/client.h>
#include <vector>
#include <string>
#include <memory>

class CompositePass : public IRenderPass {
public:
    enum class BlendMode {
        Alpha = 0,
        Additive = 1,
        Multiply = 2,
        Screen = 3
    };

    struct Layer {
        std::string textureName = "SceneColor";
        BlendMode blendMode = BlendMode::Alpha;
        float opacity = 1.0f;
        bool enabled = true;
    };

    CompositePass(const std::string& outputTexName = "CompositeFinal");
    virtual ~CompositePass() = default;

    virtual void Init(ID3D12Device* pDevice) override;
    virtual void Execute(const RenderContext& ctx) override;
    virtual std::string GetName() const override { return m_customName.empty() ? "Composite Pass" : m_customName; }

    void SetCustomName(const std::string& name) { m_customName = name; }
    const std::string& GetCustomName() const { return m_customName; }

    void AddLayer(const Layer& layer) { m_layers.push_back(layer); }
    void RemoveLayer(size_t index);
    void ClearLayers() { m_layers.clear(); }
    std::vector<Layer>& GetLayers() { return m_layers; }
    const std::vector<Layer>& GetLayers() const { return m_layers; }

    RenderTexture* GetOutputTexture() const { return m_pOutputTex.get(); }

private:
    std::string m_customName;
    std::vector<Layer> m_layers;
    std::unique_ptr<RenderTexture> m_pOutputTex;

    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoCopy;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoAlpha;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoAdditive;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoMultiply;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_psoScreen;
};
