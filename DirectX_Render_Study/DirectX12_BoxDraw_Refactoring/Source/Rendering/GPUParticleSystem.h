#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

struct GPUParticle
{
    DirectX::XMFLOAT3 position;
    float  life;        // Remaining life in seconds
    DirectX::XMFLOAT3 velocity;
    float  maxLife;     // Max life
    DirectX::XMFLOAT4 color;
    float  scale;
    UINT   active;
    DirectX::XMFLOAT2 pad;
};

class GPUParticleSystem
{
public:
    GPUParticleSystem();
    ~GPUParticleSystem();

    GPUParticleSystem(const GPUParticleSystem&) = delete;
    GPUParticleSystem& operator=(const GPUParticleSystem&) = delete;

    bool Init(ID3D12Device* device, UINT maxParticles = 16384);
    void UpdateAndSimulate(ID3D12GraphicsCommandList* cmdList, float deltaTime);
    void Render(ID3D12GraphicsCommandList* cmdList, const DirectX::XMMATRIX& view, const DirectX::XMMATRIX& proj);

    // Getters and Setters for Editor UI
    bool IsEnabled() const { return m_isEnabled; }
    void SetEnabled(bool enabled) { m_isEnabled = enabled; }

    UINT GetMaxParticles() const { return m_maxParticles; }
    
    DirectX::XMFLOAT3& GetEmitterPosition() { return m_emitterPosition; }
    void SetEmitterPosition(const DirectX::XMFLOAT3& pos) { m_emitterPosition = pos; }

    DirectX::XMFLOAT3& GetGravity() { return m_gravity; }
    void SetGravity(const DirectX::XMFLOAT3& g) { m_gravity = g; }

    DirectX::XMFLOAT4& GetParticleColor() { return m_particleColor; }
    void SetParticleColor(const DirectX::XMFLOAT4& color) { m_particleColor = color; }

    float& GetParticleScale() { return m_particleScale; }
    void SetParticleScale(float scale) { m_particleScale = scale; }

    float& GetInitialSpeed() { return m_initialSpeed; }
    void SetInitialSpeed(float speed) { m_initialSpeed = speed; }

    float& GetEmitRate() { return m_emitRate; }
    void SetEmitRate(float rate) { m_emitRate = rate; }

    void ResetParticles(ID3D12GraphicsCommandList* cmdList);

private:
    bool CreateBuffers(ID3D12Device* device);
    bool CreateSimulationPipeline(ID3D12Device* device);
    bool CreateRenderPipeline(ID3D12Device* device);

private:
    UINT m_maxParticles = 16384;
    bool m_isEnabled = true;

    // Parameters
    DirectX::XMFLOAT3 m_emitterPosition = { 0.0f, 1.0f, 0.0f };
    DirectX::XMFLOAT3 m_gravity = { 0.0f, -2.0f, 0.0f };
    DirectX::XMFLOAT4 m_particleColor = { 1.0f, 0.65f, 0.2f, 1.0f };
    float m_particleScale = 0.35f;
    float m_initialSpeed = 5.0f;
    float m_emitRate = 800.0f; // particles per second

    float m_emitAccumulator = 0.0f;
    float m_totalTime = 0.0f;

    // GPU Resource
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pParticleBuffer;
    D3D12_RESOURCE_STATES m_currentState = D3D12_RESOURCE_STATE_COMMON;

    // Descriptors
    D3D12_CPU_DESCRIPTOR_HANDLE m_uavCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_uavGPU{};
    D3D12_CPU_DESCRIPTOR_HANDLE m_srvCPU{};
    D3D12_GPU_DESCRIPTOR_HANDLE m_srvGPU{};

    // Compute Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pCSRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pCSPipelineState;

    // Render Pipeline
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pRenderRootSignature;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pRenderPipelineState;
};
