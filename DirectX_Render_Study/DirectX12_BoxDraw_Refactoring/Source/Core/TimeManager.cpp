#include "TimeManager.h"
#include <algorithm>

TimeManager::TimeManager()
{
	m_frequency.QuadPart = 0;
	m_prevTime.QuadPart = 0;
	m_currTime.QuadPart = 0;
}

void TimeManager::Init(float targetFPS)
{
	QueryPerformanceFrequency(&m_frequency);
	QueryPerformanceCounter(&m_prevTime);
	SetTargetFPS(targetFPS);
}

void TimeManager::SetTargetFPS(float targetFPS)
{
	m_targetFPS = targetFPS;
	if (m_targetFPS > 0.0f)
	{
		m_targetFrameTime = 1.0f / m_targetFPS;
	}
	else
	{
		m_targetFrameTime = 0.0f;
	}
}

void TimeManager::Update()
{
	if (m_frequency.QuadPart == 0) return;

	QueryPerformanceCounter(&m_currTime);

	float frameTime = static_cast<float>(m_currTime.QuadPart - m_prevTime.QuadPart) / static_cast<float>(m_frequency.QuadPart);

	if (m_targetFrameTime > 0.0f && frameTime < m_targetFrameTime)
	{
		float sleepTimeMs = (m_targetFrameTime - frameTime) * 1000.0f;
		if (sleepTimeMs >= 1.0f)
		{
			Sleep(static_cast<DWORD>(sleepTimeMs - 0.5f));
		}

		do
		{
			QueryPerformanceCounter(&m_currTime);
			frameTime = static_cast<float>(m_currTime.QuadPart - m_prevTime.QuadPart) / static_cast<float>(m_frequency.QuadPart);
		} while (frameTime < m_targetFrameTime);
	}

	m_deltaTime = frameTime;
	m_prevTime = m_currTime;

	if (m_deltaTime > 0.1f)
	{
		m_deltaTime = 0.1f;
	}

	m_frameCount += 1.0f;
	m_fpsTimer += m_deltaTime;
	if (m_fpsTimer >= 1.0f)
	{
		m_currentFPS = m_frameCount / m_fpsTimer;
		m_frameCount = 0.0f;
		m_fpsTimer = 0.0f;
	}
}