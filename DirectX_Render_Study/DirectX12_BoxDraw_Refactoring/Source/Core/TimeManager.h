#pragma once
#include <windows.h>

class TimeManager
{
public:
	static TimeManager& GetInstance()
	{
		static TimeManager instance;
		return instance;
	}

	void Init(float targetFPS = 60.0f);
	void Update();

	float GetDeltaTime() const { return m_deltaTime * m_timeScale; }
	float GetUnscaledDeltaTime() const { return m_deltaTime; }
	float GetFPS() const { return m_currentFPS; }

	void SetTargetFPS(float targetFPS);
	void SetTimeScale(float scale) { m_timeScale = scale >= 0.0f ? scale : 0.0f; }
	float GetTimeScale() const { return m_timeScale; }

private:
	TimeManager();
	~TimeManager() = default;

	TimeManager(const TimeManager&) = delete;
	TimeManager& operator=(const TimeManager&) = delete;

	LARGE_INTEGER m_frequency;
	LARGE_INTEGER m_prevTime;
	LARGE_INTEGER m_currTime;

	float m_targetFPS = 60.0f;
	float m_targetFrameTime = 1.0f / 60.0f;
	float m_deltaTime = 0.01667f;
	float m_timeScale = 1.0f;
	float m_currentFPS = 60.0f;

	float m_frameCount = 0.0f;
	float m_fpsTimer = 0.0f;
};