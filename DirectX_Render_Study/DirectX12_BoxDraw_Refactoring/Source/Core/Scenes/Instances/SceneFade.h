#pragma once
#include "../Base/Scene.h"
#include <memory>

enum class FadeState
{
	None,
	FadeIn,
	FadeOut
};

class SceneFade : public CScene
{
public:
	SceneFade();
	~SceneFade() = default;

	void Init() override;
	void Update() override;
	void Draw() override;

	void StartFadeIn(float duration = 0.5f);
	void StartFadeOut(float duration = 0.5f);

	bool IsFading() const { return m_fadeState != FadeState::None; }
	bool IsFadeOutComplete() const { return m_fadeState == FadeState::None && m_alpha >= 1.0f; }
	bool IsFadeInComplete() const { return m_fadeState == FadeState::None && m_alpha <= 0.0f; }

	float GetAlpha() const { return m_alpha; }
	FadeState GetFadeState() const { return m_fadeState; }

private:
	FadeState m_fadeState = FadeState::None;
	float m_alpha = 0.0f;
	float m_duration = 0.5f;
	float m_timer = 0.0f;
};