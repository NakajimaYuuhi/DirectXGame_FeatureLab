#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include "ShadowMap.h"

struct DirectionalLight
{
    DirectX::XMFLOAT4 direction = { -0.577f, -0.577f, 0.577f, 1.0f }; // xyz: direction, w: intensity
    DirectX::XMFLOAT4 color     = { 1.0f, 0.98f, 0.92f, 1.0f };        // rgb: color, a: unused
    DirectX::XMFLOAT4 ambient   = { 0.25f, 0.28f, 0.35f, 32.0f };      // rgb: ambient, a: specular power
};

struct PointLightGPUData
{
    DirectX::XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 0.0f }; // xyz: pos, w: range
    DirectX::XMFLOAT4 color    = { 1.0f, 1.0f, 1.0f, 1.0f }; // rgb: col, w: intensity
};

enum class ShadowMode : int
{
    Off        = 0, // シャドウOFF
    Projective = 1, // 投影シャドウ (深度比較なし・シルエット投影)
    HardDepth  = 2, // 深度シャドウ (1-tap ハードシャドウ)
    PCF        = 3, // PCFソフトシャドウ (3x3 / 5x5 / 16-tap Poisson Disk)
    PCSS       = 4, // PCSS (Contact-Hardening: 接地硬化・距離ソフトシャドウ)
    CSM        = 5, // カスケードシャドウマップ (3-Cascade Near/Mid/Far + 分割可視化)
    Blob       = 6  // 丸影 (Blob Shadow: 比較用簡易足元影)
};

// 768-byte aligned Constant Buffer Data for register(b1)
struct LightBufferData
{
    DirectX::XMMATRIX lightViewProj;     // 16 floats (0..15)  Cascade 0 / Single LightViewProj
    DirectX::XMFLOAT4 cameraPos;         // 4 floats  (16..19)
    DirectX::XMFLOAT4 lightDir;          // 4 floats  (20..23) xyz: dir, w: intensity
    DirectX::XMFLOAT4 lightColor;        // 4 floats  (24..27) rgb: col, w: unused
    DirectX::XMFLOAT4 ambientColor;      // 4 floats  (28..31) rgb: ambient, w: specularPower
    DirectX::XMFLOAT4 shadowParams;      // 4 floats  (32..35) x: bias, y: darkness, z: mapSize, w: shadowMode (0..6)
    DirectX::XMFLOAT4 lightCounts;       // 4 floats  (36..39) x: pointLightCount, y: blobCasterCount, z: visualizeCascades, w: projectiveGroundOnly
    DirectX::XMFLOAT4 shadowExtraParams; // 4 floats  (40..43) x: normalBias, y: pcssLightSize, z: pcfKernelMode, w: selfShadowStrength
    DirectX::XMFLOAT4 cascadeSplits;     // 4 floats  (44..47) x: split0Dist, y: split1Dist, z: split2Dist, w: orthoSize
    DirectX::XMFLOAT4 blobCasters[4];    // 16 floats (48..63) xyz: caster world pos, w: radius (completes 256-byte first block)

    PointLightGPUData pointLights[8];    // 8 point lights (32 bytes each = 256 bytes, total 512 bytes)

    DirectX::XMMATRIX lightViewProjCascade1; // 16 floats (128..143) Cascade 1 (Mid)
    DirectX::XMMATRIX lightViewProjCascade2; // 16 floats (144..159) Cascade 2 (Far)
    DirectX::XMFLOAT4 csmPadding[8];         // 32 floats (160..191) Pad to 768 bytes (multiple of 256)
};

class LightManager
{
public:
    static LightManager& GetInstance()
    {
        static LightManager instance;
        return instance;
    }

    void Init(ID3D12Device* device);
    void Finalize();
    void UpdateBuffer(const DirectX::XMFLOAT3& targetPos, const DirectX::XMFLOAT3& cameraPos);

    const DirectionalLight& GetDirectionalLight() const { return m_dirLight; }
    DirectionalLight& GetDirectionalLight() { return m_dirLight; }
    void SetDirectionalLight(const DirectionalLight& light) { m_dirLight = light; }

    void SetLightDirection(float x, float y, float z);
    void SetLightDirection(const DirectX::XMFLOAT3& dir);
    void SetLightColor(float r, float g, float b, float intensity = 1.0f);
    void SetAmbientColor(float r, float g, float b, float specularPower = 32.0f);

    // Rotation angles for easy editor manipulation
    float GetPitch() const { return m_pitch; }
    float GetYaw() const { return m_yaw; }
    void SetAngles(float pitch, float yaw);

    // Shadow Settings & Accessors
    DirectX::XMMATRIX GetLightViewProjMatrix(int cascadeIdx = 0) const
    {
        if (cascadeIdx < 0 || cascadeIdx >= 3) cascadeIdx = 0;
        return m_lightViewProjCascades[cascadeIdx];
    }
    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress() const;
    ShadowMap* GetShadowMap(int cascadeIdx = 0) const
    {
        if (cascadeIdx < 0 || cascadeIdx >= 3) cascadeIdx = 0;
        return m_shadowMaps[cascadeIdx].get();
    }

    ShadowMode GetShadowMode() const { return m_shadowEnabled ? m_shadowMode : ShadowMode::Off; }
    void SetShadowMode(ShadowMode mode)
    {
        m_shadowMode = mode;
        m_shadowEnabled = (mode != ShadowMode::Off);
    }

    float GetOrthoSize() const { return m_orthoSize; }
    void SetOrthoSize(float size) { m_orthoSize = size; }

    float GetShadowBias() const { return m_shadowBias; }
    void SetShadowBias(float bias) { m_shadowBias = bias; }

    float GetNormalBias() const { return m_normalBias; }
    void SetNormalBias(float bias) { m_normalBias = bias; }

    float GetShadowDarkness() const { return m_shadowDarkness; }
    void SetShadowDarkness(float darkness) { m_shadowDarkness = darkness; }

    bool IsShadowEnabled() const { return m_shadowEnabled && m_shadowMode != ShadowMode::Off; }
    void SetShadowEnabled(bool enabled)
    {
        m_shadowEnabled = enabled;
        if (enabled && m_shadowMode == ShadowMode::Off)
        {
            m_shadowMode = ShadowMode::PCF;
        }
    }

    // Advanced Shadow Settings (PCF / PCSS / CSM / Projective / Ground Caster)
    int GetPCFKernelMode() const { return m_pcfKernelMode; } // 0: 3x3, 1: 5x5, 2: 16-tap Poisson Disk
    void SetPCFKernelMode(int mode) { m_pcfKernelMode = mode; }

    float GetPCSSLightSize() const { return m_pcssLightSize; }
    void SetPCSSLightSize(float size) { m_pcssLightSize = size; }

    float GetSelfShadowStrength() const { return m_selfShadowStrength; }
    void SetSelfShadowStrength(float s) { m_selfShadowStrength = s; }

    float GetCascadeOrthoSize(int idx) const { return (idx >= 0 && idx < 3) ? m_cascadeOrthoSizes[idx] : m_orthoSize; }
    void SetCascadeOrthoSize(int idx, float size) { if (idx >= 0 && idx < 3) m_cascadeOrthoSizes[idx] = size; }

    bool IsVisualizeCascadesEnabled() const { return m_visualizeCascades; }
    void SetVisualizeCascadesEnabled(bool enabled) { m_visualizeCascades = enabled; }

    bool IsProjectiveGroundOnly() const { return m_projectiveGroundOnly; }
    void SetProjectiveGroundOnly(bool groundOnly) { m_projectiveGroundOnly = groundOnly; }

    bool IsGroundCastShadowEnabled() const { return m_groundCastShadow; }
    void SetGroundCastShadowEnabled(bool enabled) { m_groundCastShadow = enabled; }

    // Bloom Settings & Accessors
    bool IsBloomEnabled() const { return m_bloomEnabled; }
    void SetBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }

    float GetBloomThreshold() const { return m_bloomThreshold; }
    void SetBloomThreshold(float threshold) { m_bloomThreshold = threshold; }

    float GetBloomIntensity() const { return m_bloomIntensity; }
    void SetBloomIntensity(float intensity) { m_bloomIntensity = intensity; }

    float GetBloomSpread() const { return m_bloomSpread; }
    void SetBloomSpread(float spread) { m_bloomSpread = spread; }

    // Additional PostProcess Effects
    int GetPostProcessEffectType() const { return m_postProcessType; } // 0: None, 1: Grayscale, 2: Sepia, 3: Invert, 4: Vignette
    void SetPostProcessEffectType(int type) { m_postProcessType = type; }

    float GetPostProcessEffectStrength() const { return m_postProcessStrength; }
    void SetPostProcessEffectStrength(float strength) { m_postProcessStrength = strength; }

    // Chromatic Aberration
    bool IsChromaticAberrationEnabled() const { return m_chromaticAberrationEnabled; }
    void SetChromaticAberrationEnabled(bool enabled) { m_chromaticAberrationEnabled = enabled; }
    float GetChromaticAberrationIntensity() const { return m_chromaticAberrationIntensity; }
    void SetChromaticAberrationIntensity(float intensity) { m_chromaticAberrationIntensity = intensity; }

    // Outline / Edge Detection
    bool IsOutlineEnabled() const { return m_outlineEnabled; }
    void SetOutlineEnabled(bool enabled) { m_outlineEnabled = enabled; }
    float GetOutlineIntensity() const { return m_outlineIntensity; }
    void SetOutlineIntensity(float intensity) { m_outlineIntensity = intensity; }
    float GetOutlineThreshold() const { return m_outlineThreshold; }
    void SetOutlineThreshold(float threshold) { m_outlineThreshold = threshold; }
    float GetOutlineWidth() const { return m_outlineWidth; }
    void SetOutlineWidth(float width) { m_outlineWidth = width; }

    // Tonemapping & Exposure
    int GetToneMapType() const { return m_toneMapType; } // 0: None, 1: Reinhard, 2: ACES Filmic
    void SetToneMapType(int type) { m_toneMapType = type; }
    float GetExposure() const { return m_exposure; }
    void SetExposure(float exposure) { m_exposure = exposure; }

    // Point Light Registration (Accumulated per frame from active Point Lights)
    void ClearPointLights();
    void AddPointLight(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& color, float intensity, float range);
    size_t GetPointLightCount() const { return m_activePointLights.size(); }

private:
    LightManager();
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    void UpdateDirectionFromAngles();

    DirectionalLight m_dirLight;
    float m_pitch = 45.0f;  // Downward angle
    float m_yaw   = -45.0f; // Horizontal angle

    // Shadow mapping parameters
    ShadowMode m_shadowMode          = ShadowMode::PCF;
    float      m_orthoSize           = 22.0f;   // Width and height of single shadow map coverage
    float      m_cascadeOrthoSizes[3]= { 10.0f, 28.0f, 75.0f }; // Near / Mid / Far CSM coverage
    float      m_shadowNear          = 1.0f;
    float      m_shadowFar           = 80.0f;
    float      m_shadowDistance      = 35.0f;   // Distance from target to light camera
    float      m_shadowBias          = 0.00025f;// Small depth bias (prevents Peter Panning on character self-shadows)
    float      m_normalBias          = 0.006f;  // Normal offset bias (prevents shadow acne on curved surfaces)
    float      m_shadowDarkness      = 0.35f;   // Factor in shadow (0: pitch black, 1: no shadow)
    bool       m_shadowEnabled       = true;
    int        m_pcfKernelMode       = 2;       // 0: 3x3, 1: 5x5, 2: 16-tap Poisson Disk
    float      m_pcssLightSize       = 1.6f;    // Virtual light size for PCSS blocker/penumbra
    float      m_selfShadowStrength  = 0.75f;   // Contrast enhancement for cast shadows on 3D models
    bool       m_visualizeCascades   = false;   // Overlay RGB tint for CSM splits
    bool       m_projectiveGroundOnly= true;    // Projective shadow mode: receiver is ground only vs all
    bool       m_groundCastShadow    = false;   // Whether ground plane writes to shadow depth map

    // Bloom parameters
    bool  m_bloomEnabled    = true;
    float m_bloomThreshold  = 0.8f;
    float m_bloomIntensity  = 1.0f;
    float m_bloomSpread     = 1.0f;

    // PostProcess parameters
    int   m_postProcessType     = 0;
    float m_postProcessStrength = 1.0f;
    bool  m_chromaticAberrationEnabled   = false;
    float m_chromaticAberrationIntensity = 0.012f;
    bool  m_outlineEnabled               = false;
    float m_outlineIntensity             = 1.0f;
    float m_outlineThreshold             = 0.1f;
    float m_outlineWidth                 = 1.0f;
    int   m_toneMapType                  = 0;
    float m_exposure                     = 1.0f;

    DirectX::XMMATRIX m_lightViewProj = DirectX::XMMatrixIdentity();
    DirectX::XMMATRIX m_lightViewProjCascades[3] = {
        DirectX::XMMatrixIdentity(),
        DirectX::XMMatrixIdentity(),
        DirectX::XMMatrixIdentity()
    };
    bool m_isInitialized = false;

    // GPU Resources (3 contiguous ShadowMaps for t2, t3, t4)
    std::unique_ptr<ShadowMap> m_shadowMaps[3];
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    LightBufferData* m_mappedBuffer = nullptr;

    std::vector<PointLightGPUData> m_activePointLights;
};