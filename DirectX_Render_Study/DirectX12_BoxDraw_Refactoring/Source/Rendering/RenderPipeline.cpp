#include "RenderPipeline.h"
#include "ShadowMapPass.h"
#include "ForwardRenderPass.h"
#include "GPUParticlePass.h"
#include "PostProcessPass.h"
#include "SampleComputePass.h"
#include "CompositePass.h"
#include "RenderLayer.h"
#include "ViewportUI.h"
#include "BasicSettings.h"
#include "LightManager.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>

using json = nlohmann::json;

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

    // Ensure all Blackboard textures are in PIXEL_SHADER_RESOURCE state for ImGui thumbnails & Viewport preview
    if (localCtx.cmdList) {
        auto texNames = m_blackboard.GetTextureNames();
        for (const auto& name : texNames) {
            if (RenderTexture* pTex = m_blackboard.GetTexture(name)) {
                pTex->Transition(localCtx.cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
            }
        }

        if (!m_previewTextureName.empty()) {
            if (RenderTexture* pPreview = m_blackboard.GetTexture(m_previewTextureName)) {
                CViewportUI::GetInstance().SetTextureSRV(pPreview->GetSRV());
            }
        }
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

void RenderPipeline::SetupSimpleDefaultPipeline(ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex)
{
    ClearPasses();
    if (pDefaultSceneTex) {
        m_blackboard.RegisterTextureRef("SceneColor", pDefaultSceneTex);
    }

    AddPass(std::make_unique<ShadowMapPass>(), pDevice);
    AddPass(std::make_unique<ForwardRenderPass>(pDefaultSceneTex), pDevice);
    AddPass(std::make_unique<GPUParticlePass>(pDefaultSceneTex), pDevice);
    AddPass(std::make_unique<PostProcessPass>(pDefaultSceneTex), pDevice);

    auto cs = std::make_unique<SampleComputePass>();
    cs->SetEnabled(false);
    AddPass(std::move(cs), pDevice);
}

void RenderPipeline::SetupCommercialMultiLayerPipeline(ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex)
{
    ClearPasses();

    if (pDefaultSceneTex) {
        m_blackboard.RegisterTextureRef("SceneColor", pDefaultSceneTex);
    }

    const float skyClear[4]   = { 0.1f, 0.2f, 0.4f, 1.0f };
    const float alphaClear[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    const float blackClear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

    m_blackboard.CreateOrGetTexture("Layer_Ground",       pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, skyClear);
    m_blackboard.CreateOrGetTexture("Layer_Character",    pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, alphaClear);
    m_blackboard.CreateOrGetTexture("Layer_CharOutlined", pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, alphaClear);
    m_blackboard.CreateOrGetTexture("Layer_Particle",     pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, alphaClear);
    m_blackboard.CreateOrGetTexture("Scene_3D_Combined",  pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, blackClear);
    m_blackboard.CreateOrGetTexture("Scene_PostProcessed",pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, blackClear);
    m_blackboard.CreateOrGetTexture("Layer_UI",           pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, alphaClear);
    m_blackboard.CreateOrGetTexture("Scene_With_UI",      pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, blackClear);

    // 1. Shadow Map Pass
    AddPass(std::make_unique<ShadowMapPass>(), pDevice);

    // 2. Forward Pass (Ground & Background) -> Layer_Ground (Clears Color & Depth)
    auto groundPass = std::make_unique<ForwardRenderPass>(pDefaultSceneTex);
    groundPass->SetCustomName("Forward Pass (Ground & BG)");
    groundPass->SetOutputResourceName("Layer_Ground");
    groundPass->SetObjectFilterPreset(ForwardRenderPass::ObjectFilterPreset::GroundAndBackground);
    groundPass->SetLayerFilter(static_cast<int>(RenderLayer::Opaque));
    groundPass->SetClearColorEnabled(true);
    groundPass->SetClearDepthEnabled(true);
    AddPass(std::move(groundPass), pDevice);

    // 3. Forward Pass (Characters) -> Layer_Character (Clears Color to Alpha=0, Shares Depth from Ground)
    auto charPass = std::make_unique<ForwardRenderPass>(pDefaultSceneTex);
    charPass->SetCustomName("Forward Pass (Characters)");
    charPass->SetOutputResourceName("Layer_Character");
    charPass->SetObjectFilterPreset(ForwardRenderPass::ObjectFilterPreset::Characters);
    charPass->SetLayerFilter(static_cast<int>(RenderLayer::Opaque));
    charPass->SetClearColorEnabled(true);
    charPass->SetClearDepthEnabled(false);
    AddPass(std::move(charPass), pDevice);

    // 4. PostProcess Pass (Character Outline Only) -> Layer_CharOutlined
    auto charOutlinePass = std::make_unique<PostProcessPass>(pDefaultSceneTex);
    charOutlinePass->SetCustomName("PostProcess (Char Outline)");
    charOutlinePass->SetInputResourceName("Layer_Character");
    charOutlinePass->SetOutputResourceName("Layer_CharOutlined");
    {
        auto& s = charOutlinePass->GetSettings();
        s.useCustomSettings = true;
        s.bloomEnabled = false;
        s.outlineEnabled = true;
        s.outlineIntensity = 1.0f;
        s.outlineThreshold = 0.08f;
        s.outlineWidth = 0.5f;
        s.outlineColor[0] = 0.0f;
        s.outlineColor[1] = 0.0f;
        s.outlineColor[2] = 0.0f;
        s.preserveAlpha = true;
        s.toneMapType = 0;
        s.exposure = 1.0f;
        s.effectType = 0;
        s.outputToBackBuffer = false;
    }
    AddPass(std::move(charOutlinePass), pDevice);

    // 5. GPU Particle Pass (Compute Shader Simulation & Additive Render) -> Layer_Particle
    auto particlePass = std::make_unique<GPUParticlePass>(pDefaultSceneTex);
    particlePass->SetCustomName("GPU Particle Pass (CS)");
    particlePass->SetOutputResourceName("Layer_Particle");
    particlePass->SetClearColorEnabled(true);
    AddPass(std::move(particlePass), pDevice);

    // 6. Composite Pass (3D Scene Merge: Ground + Outlined Characters + Particles) -> Scene_3D_Combined
    auto comp3D = std::make_unique<CompositePass>("Scene_3D_Combined");
    comp3D->SetCustomName("Composite (3D Scene Merge)");
    comp3D->ClearLayers();
    comp3D->AddLayer({ "Layer_Ground",       CompositePass::BlendMode::Alpha,    1.0f, true });
    comp3D->AddLayer({ "Layer_CharOutlined", CompositePass::BlendMode::Alpha,    1.0f, true });
    comp3D->AddLayer({ "Layer_Particle",     CompositePass::BlendMode::Additive, 1.0f, true });
    AddPass(std::move(comp3D), pDevice);

    // 7. PostProcess Pass (3D Global Effect: Bloom + ACES Tonemapping) -> Scene_PostProcessed
    auto global3DPP = std::make_unique<PostProcessPass>(pDefaultSceneTex);
    global3DPP->SetCustomName("PostProcess (3D Bloom & ACES)");
    global3DPP->SetInputResourceName("Scene_3D_Combined");
    global3DPP->SetOutputResourceName("Scene_PostProcessed");
    {
        auto& s = global3DPP->GetSettings();
        s.useCustomSettings = true;
        s.bloomEnabled = true;
        s.bloomThreshold = 0.75f;
        s.bloomIntensity = 1.2f;
        s.bloomSpread = 1.2f;
        s.outlineEnabled = false;
        s.preserveAlpha = false;
        s.chromaticAberrationEnabled = false;
        s.chromaticAberrationIntensity = 0.004f;
        s.toneMapType = 2; // ACES Filmic
        s.exposure = 1.1f;
        s.effectType = 0;
        s.outputToBackBuffer = false;
    }
    AddPass(std::move(global3DPP), pDevice);

    // 8. Forward Pass (UI Layer Only - Unaffected by 3D Bloom) -> Layer_UI
    auto uiPass = std::make_unique<ForwardRenderPass>(pDefaultSceneTex);
    uiPass->SetCustomName("Forward Pass (UI Layer)");
    uiPass->SetOutputResourceName("Layer_UI");
    uiPass->SetObjectFilterPreset(ForwardRenderPass::ObjectFilterPreset::UIOnly);
    uiPass->SetClearColorEnabled(true);
    uiPass->SetClearDepthEnabled(false);
    AddPass(std::move(uiPass), pDevice);

    // 9. Composite Pass (3D + UI Merge) -> Scene_With_UI
    auto compUI = std::make_unique<CompositePass>("Scene_With_UI");
    compUI->SetCustomName("Composite (3D + UI Merge)");
    compUI->ClearLayers();
    compUI->AddLayer({ "Scene_PostProcessed", CompositePass::BlendMode::Alpha, 1.0f, true });
    compUI->AddLayer({ "Layer_UI",            CompositePass::BlendMode::Alpha, 1.0f, true });
    AddPass(std::move(compUI), pDevice);

    // 10. PostProcess Pass (Final Screen Effect: Screen Fade / Invert / Vignette) -> FinalColor
    auto finalScreenPP = std::make_unique<PostProcessPass>(pDefaultSceneTex);
    finalScreenPP->SetCustomName("PostProcess (Final Screen FX)");
    finalScreenPP->SetInputResourceName("Scene_With_UI");
    finalScreenPP->SetOutputResourceName("FinalColor");
    {
        auto& s = finalScreenPP->GetSettings();
        s.useCustomSettings = true;
        s.bloomEnabled = false;
        s.outlineEnabled = false;
        s.preserveAlpha = false;
        s.toneMapType = 0;
        s.exposure = 1.0f;
        s.effectType = 0;
        s.fade = 0.0f;
        s.fadeColor[0] = 0.0f;
        s.fadeColor[1] = 0.0f;
        s.fadeColor[2] = 0.0f;
        s.outputToBackBuffer = true;
    }
    AddPass(std::move(finalScreenPP), pDevice);

    // 11. Optional Custom Compute Shader Pass -> ComputeOutput (disabled by default)
    auto sampleCS = std::make_unique<SampleComputePass>();
    sampleCS->SetEnabled(false);
    AddPass(std::move(sampleCS), pDevice);
}

bool RenderPipeline::SaveToJson(const std::string& filePath) const
{
    try {
        json root;
        root["previewTexture"] = m_previewTextureName;

        // 0. Serialize global LightManager Post-Process & Shadow settings
        {
            const auto& lm = LightManager::GetInstance();
            json gj;
            gj["bloomEnabled"] = lm.IsBloomEnabled();
            gj["bloomThreshold"] = lm.GetBloomThreshold();
            gj["bloomIntensity"] = lm.GetBloomIntensity();
            gj["bloomSpread"] = lm.GetBloomSpread();
            gj["outlineEnabled"] = lm.IsOutlineEnabled();
            gj["outlineIntensity"] = lm.GetOutlineIntensity();
            gj["outlineThreshold"] = lm.GetOutlineThreshold();
            gj["outlineWidth"] = lm.GetOutlineWidth();
            gj["chromaticAberrationEnabled"] = lm.IsChromaticAberrationEnabled();
            gj["chromaticAberrationIntensity"] = lm.GetChromaticAberrationIntensity();
            gj["effectType"] = lm.GetPostProcessEffectType();
            gj["toneMapType"] = lm.GetToneMapType();
            gj["exposure"] = lm.GetExposure();
            gj["shadowEnabled"] = lm.IsShadowEnabled();
            gj["orthoSize"] = lm.GetOrthoSize();
            gj["shadowBias"] = lm.GetShadowBias();
            gj["shadowDarkness"] = lm.GetShadowDarkness();
            root["globalPostProcess"] = gj;
        }

        // 1. Serialize owned Blackboard textures
        json texArray = json::array();
        for (const auto& name : m_blackboard.GetTextureNames()) {
            if (!m_blackboard.IsOwnedTexture(name)) continue;
            RenderTexture* tex = m_blackboard.GetTexture(name);
            if (!tex) continue;

            json tj;
            tj["name"] = name;
            tj["width"] = tex->GetWidth();
            tj["height"] = tex->GetHeight();
            const float* cc = tex->GetClearColor();
            tj["clearColor"] = { cc[0], cc[1], cc[2], cc[3] };
            tj["allowUAV"] = tex->HasUAV();
            texArray.push_back(tj);
        }
        root["blackboardTextures"] = texArray;

        // 2. Serialize passes
        json passArray = json::array();
        for (const auto& pass : m_passes) {
            if (!pass) continue;
            json pj;
            pj["enabled"] = pass->IsEnabled();
            pj["input"] = pass->GetInputResourceName();
            pj["output"] = pass->GetOutputResourceName();

            if (dynamic_cast<ShadowMapPass*>(pass.get())) {
                pj["type"] = "ShadowMapPass";
            }
            else if (auto* fwd = dynamic_cast<ForwardRenderPass*>(pass.get())) {
                pj["type"] = "ForwardRenderPass";
                pj["customName"] = fwd->GetCustomName();
                pj["layerFilter"] = fwd->GetLayerFilter();
                pj["filterPreset"] = static_cast<int>(fwd->GetObjectFilterPreset());
                pj["tagMask"] = fwd->GetTagMask();
                pj["clearColor"] = fwd->IsClearColorEnabled();
                pj["clearDepth"] = fwd->IsClearDepthEnabled();
            }
            else if (auto* gp = dynamic_cast<GPUParticlePass*>(pass.get())) {
                pj["type"] = "GPUParticlePass";
                pj["customName"] = gp->GetCustomName();
                pj["clearColor"] = gp->IsClearColorEnabled();
            }
            else if (auto* pp = dynamic_cast<PostProcessPass*>(pass.get())) {
                pj["type"] = "PostProcessPass";
                pj["customName"] = pp->GetCustomName();
                const auto& s = pp->GetSettings();
                json sj;
                sj["useCustomSettings"] = s.useCustomSettings;
                sj["bloomEnabled"] = s.bloomEnabled;
                sj["bloomThreshold"] = s.bloomThreshold;
                sj["bloomIntensity"] = s.bloomIntensity;
                sj["bloomSpread"] = s.bloomSpread;
                sj["outlineEnabled"] = s.outlineEnabled;
                sj["outlineIntensity"] = s.outlineIntensity;
                sj["outlineThreshold"] = s.outlineThreshold;
                sj["outlineWidth"] = s.outlineWidth;
                sj["outlineColor"] = { s.outlineColor[0], s.outlineColor[1], s.outlineColor[2] };
                sj["preserveAlpha"] = s.preserveAlpha;
                sj["chromaticAberrationEnabled"] = s.chromaticAberrationEnabled;
                sj["chromaticAberrationIntensity"] = s.chromaticAberrationIntensity;
                sj["effectType"] = s.effectType;
                sj["toneMapType"] = s.toneMapType;
                sj["exposure"] = s.exposure;
                sj["fade"] = s.fade;
                sj["fadeColor"] = { s.fadeColor[0], s.fadeColor[1], s.fadeColor[2] };
                sj["outputToBackBuffer"] = s.outputToBackBuffer;
                pj["settings"] = sj;
            }
            else if (auto* comp = dynamic_cast<CompositePass*>(pass.get())) {
                pj["type"] = "CompositePass";
                pj["customName"] = comp->GetCustomName();
                json layersJson = json::array();
                for (const auto& l : comp->GetLayers()) {
                    json lj;
                    lj["textureName"] = l.textureName;
                    lj["blendMode"] = static_cast<int>(l.blendMode);
                    lj["opacity"] = l.opacity;
                    lj["enabled"] = l.enabled;
                    layersJson.push_back(lj);
                }
                pj["layers"] = layersJson;
            }
            else if (auto* cs = dynamic_cast<SampleComputePass*>(pass.get())) {
                pj["type"] = "SampleComputePass";
                pj["customName"] = cs->GetCustomName();
                pj["speed"] = cs->GetSpeed();
                pj["preview"] = cs->IsPreviewEnabled();
            }

            passArray.push_back(pj);
        }
        root["passes"] = passArray;

        std::filesystem::path p(filePath);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }

        std::ofstream ofs(filePath);
        if (!ofs.is_open()) return false;
        ofs << root.dump(4);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool RenderPipeline::LoadFromJson(const std::string& filePath, ID3D12Device* pDevice, RenderTexture* pDefaultSceneTex)
{
    try {
        std::ifstream ifs(filePath);
        if (!ifs.is_open()) return false;

        json root;
        ifs >> root;

        ClearPasses();
        if (pDefaultSceneTex) {
            m_blackboard.RegisterTextureRef("SceneColor", pDefaultSceneTex);
        }

        m_previewTextureName = root.value("previewTexture", "");

        if (root.contains("globalPostProcess") && root["globalPostProcess"].is_object()) {
            const auto& gj = root["globalPostProcess"];
            auto& lm = LightManager::GetInstance();
            if (gj.contains("bloomEnabled")) lm.SetBloomEnabled(gj["bloomEnabled"].get<bool>());
            if (gj.contains("bloomThreshold")) lm.SetBloomThreshold(gj["bloomThreshold"].get<float>());
            if (gj.contains("bloomIntensity")) lm.SetBloomIntensity(gj["bloomIntensity"].get<float>());
            if (gj.contains("bloomSpread")) lm.SetBloomSpread(gj["bloomSpread"].get<float>());
            if (gj.contains("outlineEnabled")) lm.SetOutlineEnabled(gj["outlineEnabled"].get<bool>());
            if (gj.contains("outlineIntensity")) lm.SetOutlineIntensity(gj["outlineIntensity"].get<float>());
            if (gj.contains("outlineThreshold")) lm.SetOutlineThreshold(gj["outlineThreshold"].get<float>());
            if (gj.contains("outlineWidth")) lm.SetOutlineWidth(gj["outlineWidth"].get<float>());
            if (gj.contains("chromaticAberrationEnabled")) lm.SetChromaticAberrationEnabled(gj["chromaticAberrationEnabled"].get<bool>());
            if (gj.contains("chromaticAberrationIntensity")) lm.SetChromaticAberrationIntensity(gj["chromaticAberrationIntensity"].get<float>());
            if (gj.contains("effectType")) lm.SetPostProcessEffectType(gj["effectType"].get<int>());
            if (gj.contains("toneMapType")) lm.SetToneMapType(gj["toneMapType"].get<int>());
            if (gj.contains("exposure")) lm.SetExposure(gj["exposure"].get<float>());
            if (gj.contains("shadowEnabled")) lm.SetShadowEnabled(gj["shadowEnabled"].get<bool>());
            if (gj.contains("orthoSize")) lm.SetOrthoSize(gj["orthoSize"].get<float>());
            if (gj.contains("shadowBias")) lm.SetShadowBias(gj["shadowBias"].get<float>());
            if (gj.contains("shadowDarkness")) lm.SetShadowDarkness(gj["shadowDarkness"].get<float>());
        }

        if (root.contains("blackboardTextures") && root["blackboardTextures"].is_array()) {
            for (const auto& tj : root["blackboardTextures"]) {
                std::string name = tj.value("name", "");
                UINT w = tj.value("width", SCREEN_WIDTH);
                UINT h = tj.value("height", SCREEN_HEIGHT);
                bool allowUAV = tj.value("allowUAV", false);
                float cc[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                if (tj.contains("clearColor") && tj["clearColor"].is_array() && tj["clearColor"].size() == 4) {
                    for (int i = 0; i < 4; ++i) cc[i] = tj["clearColor"][i].get<float>();
                }
                if (!name.empty()) {
                    m_blackboard.CreateOrGetTexture(name, pDevice, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, cc, allowUAV);
                }
            }
        }

        if (root.contains("passes") && root["passes"].is_array()) {
            for (const auto& pj : root["passes"]) {
                std::string type = pj.value("type", "");
                std::unique_ptr<IRenderPass> pass = nullptr;

                if (type == "ShadowMapPass") {
                    pass = std::make_unique<ShadowMapPass>();
                }
                else if (type == "ForwardRenderPass") {
                    auto fwd = std::make_unique<ForwardRenderPass>(pDefaultSceneTex);
                    fwd->SetCustomName(pj.value("customName", ""));
                    fwd->SetObjectFilterPreset(static_cast<ForwardRenderPass::ObjectFilterPreset>(pj.value("filterPreset", 0)));
                    if (fwd->GetObjectFilterPreset() == ForwardRenderPass::ObjectFilterPreset::Custom) {
                        fwd->SetTagMask(pj.value("tagMask", 0xFFFFFFFFu));
                    }
                    fwd->SetLayerFilter(pj.value("layerFilter", -1));
                    fwd->SetClearColorEnabled(pj.value("clearColor", true));
                    fwd->SetClearDepthEnabled(pj.value("clearDepth", true));
                    pass = std::move(fwd);
                }
                else if (type == "GPUParticlePass") {
                    auto gp = std::make_unique<GPUParticlePass>(pDefaultSceneTex);
                    gp->SetCustomName(pj.value("customName", ""));
                    gp->SetClearColorEnabled(pj.value("clearColor", false));
                    pass = std::move(gp);
                }
                else if (type == "PostProcessPass") {
                    auto pp = std::make_unique<PostProcessPass>(pDefaultSceneTex);
                    pp->SetCustomName(pj.value("customName", ""));
                    if (pj.contains("settings") && pj["settings"].is_object()) {
                        const auto& sj = pj["settings"];
                        auto& s = pp->GetSettings();
                        s.useCustomSettings = sj.value("useCustomSettings", false);
                        s.bloomEnabled = sj.value("bloomEnabled", false);
                        s.bloomThreshold = sj.value("bloomThreshold", 0.8f);
                        s.bloomIntensity = sj.value("bloomIntensity", 1.2f);
                        s.bloomSpread = sj.value("bloomSpread", 1.0f);
                        s.outlineEnabled = sj.value("outlineEnabled", false);
                        s.outlineIntensity = sj.value("outlineIntensity", 1.0f);
                        s.outlineThreshold = sj.value("outlineThreshold", 0.1f);
                        s.outlineWidth = sj.value("outlineWidth", 1.5f);
                        if (sj.contains("outlineColor") && sj["outlineColor"].is_array() && sj["outlineColor"].size() == 3) {
                            for (int i = 0; i < 3; ++i) s.outlineColor[i] = sj["outlineColor"][i].get<float>();
                        }
                        s.preserveAlpha = sj.value("preserveAlpha", false);
                        s.chromaticAberrationEnabled = sj.value("chromaticAberrationEnabled", false);
                        s.chromaticAberrationIntensity = sj.value("chromaticAberrationIntensity", 0.005f);
                        s.effectType = sj.value("effectType", 0);
                        s.toneMapType = sj.value("toneMapType", 0);
                        s.exposure = sj.value("exposure", 1.0f);
                        s.fade = sj.value("fade", 0.0f);
                        if (sj.contains("fadeColor") && sj["fadeColor"].is_array() && sj["fadeColor"].size() == 3) {
                            for (int i = 0; i < 3; ++i) s.fadeColor[i] = sj["fadeColor"][i].get<float>();
                        }
                        s.outputToBackBuffer = sj.value("outputToBackBuffer", true);
                    }
                    pass = std::move(pp);
                }
                else if (type == "CompositePass") {
                    std::string outName = pj.value("output", "CompositeFinal");
                    auto comp = std::make_unique<CompositePass>(outName);
                    comp->SetCustomName(pj.value("customName", ""));
                    if (pj.contains("layers") && pj["layers"].is_array()) {
                        comp->ClearLayers();
                        for (const auto& lj : pj["layers"]) {
                            CompositePass::Layer l;
                            l.textureName = lj.value("textureName", "SceneColor");
                            l.blendMode = static_cast<CompositePass::BlendMode>(lj.value("blendMode", 0));
                            l.opacity = lj.value("opacity", 1.0f);
                            l.enabled = lj.value("enabled", true);
                            comp->AddLayer(l);
                        }
                    }
                    pass = std::move(comp);
                }
                else if (type == "SampleComputePass") {
                    auto cs = std::make_unique<SampleComputePass>();
                    cs->SetCustomName(pj.value("customName", ""));
                    cs->SetSpeed(pj.value("speed", 1.0f));
                    cs->SetPreviewEnabled(pj.value("preview", false));
                    pass = std::move(cs);
                }

                if (pass) {
                    pass->SetEnabled(pj.value("enabled", true));
                    if (pj.contains("input")) pass->SetInputResourceName(pj.value("input", ""));
                    if (pj.contains("output")) pass->SetOutputResourceName(pj.value("output", ""));
                    AddPass(std::move(pass), pDevice);
                }
            }
        }
        return true;
    }
    catch (...) {
        return false;
    }
}
