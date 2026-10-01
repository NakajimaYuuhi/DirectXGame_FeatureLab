#pragma once
#include "Component.h"
#include <string>
#include <directxmath.h>

class ParticleEmitterComponent : public CComponent
{
public:
	ParticleEmitterComponent();
	~ParticleEmitterComponent() override = default;

	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Movement; }

	void Emit();

	void SetParticlePrefab(const std::string& prefab) { m_particlePrefab = prefab; }
	std::string GetParticlePrefab() const { return m_particlePrefab; }

	void SetBurstCount(int count) { m_burstCount = count; }
	int GetBurstCount() const { return m_burstCount; }

	void SetSpawnInterval(float interval) { m_spawnInterval = interval; }
	float GetSpawnInterval() const { return m_spawnInterval; }

	void SetBurstOnStart(bool burst) { m_burstOnStart = burst; }
	bool GetBurstOnStart() const { return m_burstOnStart; }

	void SetParticleScale(const DirectX::XMFLOAT3& scale) { m_particleScale = scale; }
	DirectX::XMFLOAT3 GetParticleScale() const { return m_particleScale; }

private:
	std::string m_particlePrefab = "RandomParticle";
	int m_burstCount = 10;
	float m_spawnInterval = 0.0f;
	float m_timer = 0.0f;
	bool m_burstOnStart = true;
	bool m_hasBurstOnStart = false;
	DirectX::XMFLOAT3 m_particleScale = { 0.2f, 0.2f, 0.2f };
};
