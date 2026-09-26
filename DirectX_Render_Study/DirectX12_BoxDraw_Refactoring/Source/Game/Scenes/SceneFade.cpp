#include "SceneFade.h"
#include "TimeManager.h"
#include "imgui.h"

SceneFade::SceneFade()
	: CScene(Scenes::ID::FADE)
{
	m_Name = "FadeScene";
}

void SceneFade::Init()
{
	m_fadeState = FadeState::None;
	m_alpha = 0.0f;
}

void SceneFade::StartFadeOut(float duration)
{
	m_fadeState = FadeState::FadeOut;
	m_duration = duration > 0.0f ? duration : 0.5f;
	m_timer = 0.0f;
	m_alpha = 0.0f;
}

void SceneFade::StartFadeIn(float duration)
{
	m_fadeState = FadeState::FadeIn;
	m_duration = duration > 0.0f ? duration : 0.5f;
	m_timer = 0.0f;
	m_alpha = 1.0f;
}

void SceneFade::Update()
{
	if (m_fadeState == FadeState::None) return;

	float dt = TimeManager::GetInstance().GetDeltaTime();
	m_timer += dt;
	float progress = m_timer / m_duration;
	if (progress > 1.0f) progress = 1.0f;

	if (m_fadeState == FadeState::FadeOut)
	{
		m_alpha = progress;
		if (progress >= 1.0f)
		{
			m_alpha = 1.0f;
			m_fadeState = FadeState::None;
		}
	}
	else if (m_fadeState == FadeState::FadeIn)
	{
		m_alpha = 1.0f - progress;
		if (progress >= 1.0f)
		{
			m_alpha = 0.0f;
			m_fadeState = FadeState::None;
		}
	}
}

void SceneFade::Draw()
{
	if (m_alpha <= 0.001f) return;

	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	if (drawList)
	{
		ImU32 col = IM_COL32(0, 0, 0, static_cast<int>(m_alpha * 255.0f));
		ImGuiIO& io = ImGui::GetIO();
		drawList->AddRectFilled(ImVec2(0, 0), io.DisplaySize, col);
	}
}