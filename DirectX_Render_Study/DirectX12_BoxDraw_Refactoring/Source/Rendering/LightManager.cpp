#include "LightManager.h"
#include <cmath>
#include <stdexcept>

LightManager::LightManager()
{
    UpdateDirectionFromAngles();
}

void LightManager::Init(ID3D12Device* device)
{
    if (!device || m_isInitialized) return;

    // 1. Create Shadow Map Depth Resource (2048 x 2048)
    m_shadowMap = std::make_unique<ShadowMap>(device, 2048, 2048);

    // 2. Create Constant Buffer Resource (Upload Heap, 256 bytes)
    D3D12_HEAP_PROPERTIES heapProp = {};
    heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width = sizeof(LightBufferData);
    resDesc.Height = 1;
    resDesc.DepthOrArraySize = 1;
    resDesc.MipLevels = 1;
    resDesc.Format = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count = 1;
    resDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    HRESULT hr = device->CreateCommittedResource(
        &heapProp,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_constantBuffer)
    );
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to create LightManager Constant Buffer.");
    }

    hr = m_constantBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedBuffer));
    if (FAILED(hr))
    {
        throw std::runtime_error("Failed to map LightManager Constant Buffer.");
    }

    m_isInitialized = true;
}

void LightManager::UpdateBuffer(const DirectX::XMFLOAT3& targetPos, const DirectX::XMFLOAT3& cameraPos)
{
    // 1. Calculate Light View & Orthographic Projection Matrix
    DirectX::XMVECTOR targetV = DirectX::XMLoadFloat3(&targetPos);
    DirectX::XMVECTOR dirV    = DirectX::XMVectorSet(m_dirLight.direction.x, m_dirLight.direction.y, m_dirLight.direction.z, 0.0f);
    dirV = DirectX::XMVector3Normalize(dirV);

    DirectX::XMVECTOR lightPosV = DirectX::XMVectorSubtract(targetV, DirectX::XMVectorScale(dirV, m_shadowDistance));
    
    DirectX::XMVECTOR upV = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    if (std::abs(m_dirLight.direction.y) > 0.99f)
    {
        upV = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    }

    DirectX::XMMATRIX lightView = DirectX::XMMatrixLookAtLH(lightPosV, targetV, upV);
    DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicLH(m_orthoSize, m_orthoSize, m_shadowNear, m_shadowFar);
    m_lightViewProj = lightView * lightProj;

    // 2. Write to mapped constant buffer
    if (m_mappedBuffer)
    {
        m_mappedBuffer->lightViewProj = DirectX::XMMatrixTranspose(m_lightViewProj);
        m_mappedBuffer->cameraPos     = { cameraPos.x, cameraPos.y, cameraPos.z, 1.0f };
        m_mappedBuffer->lightDir      = m_dirLight.direction;
        m_mappedBuffer->lightColor    = m_dirLight.color;
        m_mappedBuffer->ambientColor  = m_dirLight.ambient;

        float mapSize = m_shadowMap ? static_cast<float>(m_shadowMap->GetWidth()) : 2048.0f;
        m_mappedBuffer->shadowParams  = {
            m_shadowBias,
            m_shadowDarkness,
            mapSize,
            m_shadowEnabled ? 1.0f : 0.0f
        };
    }
}

D3D12_GPU_VIRTUAL_ADDRESS LightManager::GetConstantBufferGPUAddress() const
{
    return m_constantBuffer ? m_constantBuffer->GetGPUVirtualAddress() : 0;
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

    float x = std::cos(pitchRad) * std::sin(yawRad);
    float y = -std::sin(pitchRad);
    float z = std::cos(pitchRad) * std::cos(yawRad);

    SetLightDirection(x, y, z);
}
