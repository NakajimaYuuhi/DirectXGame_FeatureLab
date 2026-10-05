#pragma once
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <memory>
#include "ShadowMap.h"

struct DirectionalLight
{
    DirectX::XMFLOAT4 direction = { -0.577f, -0.577f, 0.577f, 1.0f }; // xyz: direction, w: intensity
    DirectX::XMFLOAT4 color     = { 1.0f, 0.98f, 0.92f, 1.0f };        // rgb: color, a: unused
    DirectX::XMFLOAT4 ambient   = { 0.25f, 0.28f, 0.35f, 32.0f };      // rgb: ambient, a: specular power
};

// 256-byte aligned Constant Buffer Data for register(b1)
struct LightBufferData
{
    DirectX::XMMATRIX lightViewProj;    // 16 floats (0..15)
    DirectX::XMFLOAT4 cameraPos;        // 4 floats  (16..19)
    DirectX::XMFLOAT4 lightDir;         // 4 floats  (20..23) xyz: dir, w: intensity
    DirectX::XMFLOAT4 lightColor;       // 4 floats  (24..27) rgb: col, w: unused
    DirectX::XMFLOAT4 ambientColor;     // 4 floats  (28..31) rgb: ambient, w: specularPower
    DirectX::XMFLOAT4 shadowParams;     // 4 floats  (32..35) x: bias, y: darkness, z: mapSize, w: enabled
    float             padding[28];      // Pad to 256 bytes (64 floats total = 256 bytes)
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
    DirectX::XMMATRIX GetLightViewProjMatrix() const { return m_lightViewProj; }
    D3D12_GPU_VIRTUAL_ADDRESS GetConstantBufferGPUAddress() const;
    ShadowMap* GetShadowMap() const { return m_shadowMap.get(); }

    float GetOrthoSize() const { return m_orthoSize; }
    void SetOrthoSize(float size) { m_orthoSize = size; }

    float GetShadowBias() const { return m_shadowBias; }
    void SetShadowBias(float bias) { m_shadowBias = bias; }

    float GetShadowDarkness() const { return m_shadowDarkness; }
    void SetShadowDarkness(float darkness) { m_shadowDarkness = darkness; }

    bool IsShadowEnabled() const { return m_shadowEnabled; }
    void SetShadowEnabled(bool enabled) { m_shadowEnabled = enabled; }

    // Bloom Settings & Accessors
    bool IsBloomEnabled() const { return m_bloomEnabled; }
    void SetBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }

    float GetBloomThreshold() const { return m_bloomThreshold; }
    void SetBloomThreshold(float threshold) { m_bloomThreshold = threshold; }

    float GetBloomIntensity() const { return m_bloomIntensity; }
    void SetBloomIntensity(float intensity) { m_bloomIntensity = intensity; }

    float GetBloomSpread() const { return m_bloomSpread; }
    void SetBloomSpread(float spread) { m_bloomSpread = spread; }

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
    float m_orthoSize       = 35.0f; // Width and height of shadow coverage volume
    float m_shadowNear      = 1.0f;
    float m_shadowFar       = 80.0f;
    float m_shadowDistance  = 35.0f; // Distance from target to light camera
    float m_shadowBias      = 0.0015f;
    float m_shadowDarkness  = 0.5f;  // Factor in shadow (0: pitch black, 1: no shadow)
    bool  m_shadowEnabled   = true;

    // Bloom parameters
    bool  m_bloomEnabled    = true;
    float m_bloomThreshold  = 0.8f;
    float m_bloomIntensity  = 1.0f;
    float m_bloomSpread     = 1.0f;

    DirectX::XMMATRIX m_lightViewProj = DirectX::XMMatrixIdentity();
    bool m_isInitialized = false;

    // GPU Resources
    std::unique_ptr<ShadowMap> m_shadowMap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_constantBuffer;
    LightBufferData* m_mappedBuffer = nullptr;
};