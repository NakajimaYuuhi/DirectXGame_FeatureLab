#pragma once
#include "Component.h"
#include <DirectXMath.h>
#include <string>

class CameraComponent;
struct ImDrawList;
struct ImVec2;

enum class LightType
{
	Directional = 0,
	Point = 1,
	Spot = 2
};

class LightComponent : public CComponent
{
public:
	LightComponent();
	virtual ~LightComponent() = default;

	void Init() override;
	void Start() override;
	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::PostPhysics; }

	// Light Type
	LightType GetLightType() const { return m_type; }
	void SetLightType(LightType type) { m_type = type; }
	std::string GetLightTypeName() const;

	// Light Color & Intensity
	DirectX::XMFLOAT3 GetColor() const { return m_color; }
	void SetColor(const DirectX::XMFLOAT3& color) { m_color = color; }
	void SetColor(float r, float g, float b) { m_color = { r, g, b }; }

	float GetIntensity() const { return m_intensity; }
	void SetIntensity(float intensity) { m_intensity = intensity; }

	// Ambient & Specular
	DirectX::XMFLOAT3 GetAmbientColor() const { return m_ambientColor; }
	void SetAmbientColor(const DirectX::XMFLOAT3& ambient) { m_ambientColor = ambient; }
	void SetAmbientColor(float r, float g, float b) { m_ambientColor = { r, g, b }; }

	float GetSpecularPower() const { return m_specularPower; }
	void SetSpecularPower(float power) { m_specularPower = power; }

	// Shadows
	bool GetCastShadow() const { return m_castShadow; }
	void SetCastShadow(bool cast) { m_castShadow = cast; }

	float GetShadowBias() const { return m_shadowBias; }
	void SetShadowBias(float bias) { m_shadowBias = bias; }

	float GetShadowDarkness() const { return m_shadowDarkness; }
	void SetShadowDarkness(float darkness) { m_shadowDarkness = darkness; }

	// Point / Spot light future-ready parameters
	float GetRange() const { return m_range; }
	void SetRange(float range) { m_range = range; }

	float GetSpotAngle() const { return m_spotAngle; }
	void SetSpotAngle(float angle) { m_spotAngle = angle; }

	// Synchronize parameters to LightManager
	void SyncToLightManager();

	// Editor Gizmo Rendering
	void DrawDebug(CameraComponent* camera, ImDrawList* customDrawList, const ImVec2& vpPos, const ImVec2& vpSize);

private:
	LightType m_type = LightType::Directional;
	DirectX::XMFLOAT3 m_color = { 1.0f, 0.98f, 0.92f };
	float m_intensity = 1.0f;
	DirectX::XMFLOAT3 m_ambientColor = { 0.25f, 0.28f, 0.35f };
	float m_specularPower = 32.0f;

	bool m_castShadow = true;
	float m_shadowBias = 0.0015f;
	float m_shadowDarkness = 0.5f;

	float m_range = 10.0f;
	float m_spotAngle = 45.0f; // degrees
};
