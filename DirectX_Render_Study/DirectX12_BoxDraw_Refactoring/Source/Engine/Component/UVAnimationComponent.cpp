#include "UVAnimationComponent.h"
#include "Object.h"
#include "Transform.h"

UVAnimationComponent::UVAnimationComponent()
	: CComponent("UVAnimationComponent")
{
}

void UVAnimationComponent::SetGrid(int rows, int cols)
{
	m_rows = rows > 0 ? rows : 1;
	m_cols = cols > 0 ? cols : 1;
	m_totalFrames = m_rows * m_cols;
}

void UVAnimationComponent::Update(float deltaTime)
{
	if (!m_Owner) return;

	CTransform* transform = m_Owner->GetComponent<CTransform>();
	if (!transform) return;

	// Initial scale update
	float scaleX = 1.0f / m_cols;
	float scaleY = 1.0f / m_rows;
	transform->SetUVScale({ scaleX, scaleY });

	m_timer += deltaTime;
	if (m_timer >= m_frameDuration)
	{
		m_timer -= m_frameDuration;
		m_currentFrame++;

		if (m_currentFrame >= m_totalFrames)
		{
			if (m_loop)
			{
				m_currentFrame = 0;
			}
			else
			{
				m_currentFrame = m_totalFrames - 1;
				if (m_destroyOnComplete)
				{
					m_Owner->SetIsDestroyed(true);
					return;
				}
			}
		}
	}

	int currentRow = m_currentFrame / m_cols;
	int currentCol = m_currentFrame % m_cols;
	transform->SetUVOffset({ currentCol * scaleX, currentRow * scaleY });
}
