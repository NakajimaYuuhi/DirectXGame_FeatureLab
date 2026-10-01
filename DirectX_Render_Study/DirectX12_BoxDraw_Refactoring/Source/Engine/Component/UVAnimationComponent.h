#pragma once
#include "Component.h"
#include <directxmath.h>

class UVAnimationComponent : public CComponent
{
public:
	UVAnimationComponent();
	~UVAnimationComponent() override = default;

	void Update(float deltaTime) override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Animation; }

	void SetGrid(int rows, int cols);
	void SetTotalFrames(int count) { m_totalFrames = count; }
	void SetFrameDuration(float duration) { m_frameDuration = duration; }
	void SetLoop(bool loop) { m_loop = loop; }
	void SetDestroyOnComplete(bool destroy) { m_destroyOnComplete = destroy; }

	int GetCurrentFrame() const { return m_currentFrame; }
	int GetRows() const { return m_rows; }
	int GetCols() const { return m_cols; }
	int GetTotalFrames() const { return m_totalFrames; }
	float GetFrameDuration() const { return m_frameDuration; }
	bool IsLoop() const { return m_loop; }
	bool IsDestroyOnComplete() const { return m_destroyOnComplete; }

private:
	int m_rows = 1;
	int m_cols = 1;
	int m_totalFrames = 1;
	float m_frameDuration = 0.05f;
	float m_timer = 0.0f;
	int m_currentFrame = 0;
	bool m_loop = false;
	bool m_destroyOnComplete = true;
};
