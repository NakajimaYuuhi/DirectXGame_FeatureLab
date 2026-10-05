#pragma once
#include <DirectXMath.h>

struct DirectionalLight
{
    DirectX::XMFLOAT4 direction = { -0.577f, -0.577f, 0.577f, 1.0f }; // xyz: direction, w: intensity
    DirectX::XMFLOAT4 color     = { 1.0f, 0.98f, 0.92f, 1.0f };        // rgb: color, a: unused
    DirectX::XMFLOAT4 ambient   = { 0.25f, 0.28f, 0.35f, 32.0f };      // rgb: ambient, a: specular power
};

class LightManager
{
public:
    static LightManager& GetInstance()
    {
        static LightManager instance;
        return instance;
    }

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

private:
    LightManager();
    ~LightManager() = default;
    LightManager(const LightManager&) = delete;
    LightManager& operator=(const LightManager&) = delete;

    void UpdateDirectionFromAngles();

    DirectionalLight m_dirLight;
    float m_pitch = 45.0f;  // Downward angle
    float m_yaw   = -45.0f; // Horizontal angle
};
