#include "LightManager.h"
#include <cmath>

LightManager::LightManager()
{
    UpdateDirectionFromAngles();
}

void LightManager::SetLightDirection(float x, float y, float z)
{
    DirectX::XMVECTOR v = DirectX::XMVectorSet(x, y, z, 0.0f);
    v = DirectX::XMVector3Normalize(v);
    DirectX::XMFLOAT3 n;
    DirectX::XMStoreFloat3(&n, v);
    m_dirLight.direction.x = n.x;
    m_dirLight.direction.y = n.y;
    m_dirLight.direction.z = n.z;
}

void LightManager::SetLightDirection(const DirectX::XMFLOAT3& dir)
{
    SetLightDirection(dir.x, dir.y, dir.z);
}

void LightManager::SetLightColor(float r, float g, float b, float intensity)
{
    m_dirLight.color.x = r;
    m_dirLight.color.y = g;
    m_dirLight.color.z = b;
    m_dirLight.direction.w = intensity;
}

void LightManager::SetAmbientColor(float r, float g, float b, float specularPower)
{
    m_dirLight.ambient.x = r;
    m_dirLight.ambient.y = g;
    m_dirLight.ambient.z = b;
    m_dirLight.ambient.w = specularPower;
}

void LightManager::SetAngles(float pitch, float yaw)
{
    m_pitch = pitch;
    m_yaw = yaw;
    UpdateDirectionFromAngles();
}

void LightManager::UpdateDirectionFromAngles()
{
    float pitchRad = DirectX::XMConvertToRadians(m_pitch);
    float yawRad   = DirectX::XMConvertToRadians(m_yaw);

    // Direction vector facing downwards and forward
    float x = std::cos(pitchRad) * std::sin(yawRad);
    float y = -std::sin(pitchRad);
    float z = std::cos(pitchRad) * std::cos(yawRad);

    SetLightDirection(x, y, z);
}
