#include "LightManager.h"
#include "ObjectManager.h"
#include "Object.h"
#include "Transform.h"
#include "Model.h"
#include "LightComponent.h"
#include <cmath>
#include <stdexcept>
#include <algorithm>

LightManager::LightManager()
{
    UpdateDirectionFromAngles();
}

void LightManager::Init(ID3D12Device* device)
{
    if (!device || m_isInitialized) return;

    // 1. Create 3 Contiguous Shadow Map Depth Resources (2048 x 2048 each) for t2, t3, t4
    for (int i = 0; i < 3; ++i)
    {
        m_shadowMaps[i] = std::make_unique<ShadowMap>(device, 2048, 2048);
    }

    // 2. Create Constant Buffer Resource (Upload Heap, 768 bytes)
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

void LightManager::Finalize()
{
    if (m_constantBuffer)
    {
        if (m_mappedBuffer)
        {
            m_constantBuffer->Unmap(0, nullptr);
            m_mappedBuffer = nullptr;
        }
        m_constantBuffer.Reset();
    }
    for (int i = 0; i < 3; ++i)
    {
        m_shadowMaps[i].reset();
    }
    m_activePointLights.clear();
    m_isInitialized = false;
}

void LightManager::UpdateBuffer(const DirectX::XMFLOAT3& targetPos, const DirectX::XMFLOAT3& cameraPos)
{
    // 1. Calculate Light View & Orthographic Projection Matrices (Single + 3-Cascade CSM)
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

    ShadowMode activeMode = GetShadowMode();
    if (activeMode == ShadowMode::CSM)
    {
        for (int c = 0; c < 3; ++c)
        {
            float cSize = (std::max)(2.0f, m_cascadeOrthoSizes[c]);
            DirectX::XMMATRIX cProj = DirectX::XMMatrixOrthographicLH(cSize, cSize, m_shadowNear, m_shadowFar);
            m_lightViewProjCascades[c] = lightView * cProj;
        }
        m_lightViewProj = m_lightViewProjCascades[0];
    }
    else
    {
        float ortho = (std::max)(2.0f, m_orthoSize);
        DirectX::XMMATRIX lightProj = DirectX::XMMatrixOrthographicLH(ortho, ortho, m_shadowNear, m_shadowFar);
        m_lightViewProj = lightView * lightProj;
        m_lightViewProjCascades[0] = m_lightViewProj;
        m_lightViewProjCascades[1] = m_lightViewProj;
        m_lightViewProjCascades[2] = m_lightViewProj;
    }

    // 2. Write to mapped constant buffer
    if (m_mappedBuffer)
    {
        m_mappedBuffer->lightViewProj         = DirectX::XMMatrixTranspose(m_lightViewProjCascades[0]);
        m_mappedBuffer->lightViewProjCascade1 = DirectX::XMMatrixTranspose(m_lightViewProjCascades[1]);
        m_mappedBuffer->lightViewProjCascade2 = DirectX::XMMatrixTranspose(m_lightViewProjCascades[2]);

        m_mappedBuffer->cameraPos     = { cameraPos.x, cameraPos.y, cameraPos.z, 1.0f };
        m_mappedBuffer->lightDir      = m_dirLight.direction;
        m_mappedBuffer->lightColor    = m_dirLight.color;
        m_mappedBuffer->ambientColor  = m_dirLight.ambient;

        float mapSize = m_shadowMaps[0] ? static_cast<float>(m_shadowMaps[0]->GetWidth()) : 2048.0f;
        m_mappedBuffer->shadowParams  = {
            m_shadowBias,
            m_shadowDarkness,
            mapSize,
            static_cast<float>(static_cast<int>(activeMode))
        };

        m_mappedBuffer->shadowExtraParams = {
            m_normalBias,
            m_pcssLightSize,
            static_cast<float>(m_pcfKernelMode),
            m_selfShadowStrength
        };

        m_mappedBuffer->cascadeSplits = {
            m_cascadeOrthoSizes[0] * 0.45f,
            m_cascadeOrthoSizes[1] * 0.45f,
            m_cascadeOrthoSizes[2] * 0.45f,
            m_orthoSize
        };

        // 3. Collect active point lights and blob shadow casters from scene objects
        ClearPointLights();
        std::vector<DirectX::XMFLOAT4> blobList;

        const auto& objectList = ObjectManager::GetInstance().GetObjectList();
        for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
        {
            for (const auto& obj : objectList[tagIdx])
            {
                if (!obj || obj->GetIsDestroyed()) continue;
                LightComponent* lightComp = obj->GetComponent<LightComponent>();
                if (lightComp && lightComp->IsEnabled())
                {
                    if (lightComp->GetLightType() == LightType::Point)
                    {
                        CTransform* transform = obj->GetComponent<CTransform>();
                        DirectX::XMFLOAT3 pos = transform ? transform->GetWorldPosition() : DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
                        AddPointLight(pos, lightComp->GetColor(), lightComp->GetIntensity(), lightComp->GetRange());
                    }
                }

                // Collect up to 4 character/object casters for Blob Shadow mode
                if (blobList.size() < 4 &&
                    tagIdx != ObjectTag::FIELD && tagIdx != ObjectTag::BACKGROUND &&
                    tagIdx != ObjectTag::UI && tagIdx != ObjectTag::CAMERA &&
                    tagIdx != ObjectTag::LIGHT && tagIdx != ObjectTag::EFFECT)
                {
                    if (obj->GetComponent<CModel>())
                    {
                        CTransform* transform = obj->GetComponent<CTransform>();
                        if (transform)
                        {
                            DirectX::XMFLOAT3 pos = transform->GetWorldPosition();
                            DirectX::XMFLOAT3 scale = transform->GetScale();
                            float radius = (std::max)(0.45f, (scale.x + scale.z) * 0.65f);
                            blobList.push_back({ pos.x, pos.y, pos.z, radius });
                        }
                    }
                }
            }
        }

        for (size_t i = 0; i < 4; ++i)
        {
            m_mappedBuffer->blobCasters[i] = (i < blobList.size()) ? blobList[i] : DirectX::XMFLOAT4(0.0f, -999.0f, 0.0f, 0.0f);
        }

        // 4. Write Point Lights & Flags
        size_t numPointLights = (std::min)(m_activePointLights.size(), static_cast<size_t>(8));
        m_mappedBuffer->lightCounts = {
            static_cast<float>(numPointLights),
            static_cast<float>(blobList.size()),
            m_visualizeCascades ? 1.0f : 0.0f,
            m_projectiveGroundOnly ? 1.0f : 0.0f
        };

        for (size_t i = 0; i < numPointLights; ++i)
        {
            m_mappedBuffer->pointLights[i] = m_activePointLights[i];
        }
        for (size_t i = numPointLights; i < 8; ++i)
        {
            m_mappedBuffer->pointLights[i] = {};
        }
    }
}

void LightManager::ClearPointLights()
{
    m_activePointLights.clear();
}

void LightManager::AddPointLight(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& color, float intensity, float range)
{
    if (m_activePointLights.size() >= 8) return;

    PointLightGPUData data;
    data.position = { pos.x, pos.y, pos.z, range };
    data.color    = { color.x, color.y, color.z, intensity };
    m_activePointLights.push_back(data);
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
