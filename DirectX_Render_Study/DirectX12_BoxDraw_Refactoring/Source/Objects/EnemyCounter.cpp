#include "EnemyCounter.h"
#include "ObjectManager.h"
#include "EnemyCount.h"
#include "TextRenderer.h"
#include "EventManager.h"
#include "EventData_NextScene.h"

// コンストラクタ
EnemyCounter::EnemyCounter(String _Name) 
	: enemyCount_(0) 
	, defeatCount_(0)
	, enemyCountUI_(nullptr)
	, m_textRenderer(nullptr)
{ 
	SetName(_Name); 
}

CTextRenderer* EnemyCounter::GetTextRenderer()
{
	if (m_textRenderer && m_textRenderer->GetOwner() && !m_textRenderer->GetOwner()->GetIsDestroyed())
	{
		return m_textRenderer;
	}
	m_textRenderer = nullptr;

	const auto& objectList = ObjectManager::GetInstance().GetObjectList();
	if (static_cast<size_t>(ObjectTag::TEXT) < objectList.size())
	{
		for (const auto& obj : objectList[static_cast<size_t>(ObjectTag::TEXT)])
		{
			if (!obj || obj->GetIsDestroyed()) continue;
			auto tr = obj->GetComponent<CTextRenderer>();
			if (tr)
			{
				m_textRenderer = tr;
				return m_textRenderer;
			}
		}
	}
	return nullptr;
}

EnemyCount* EnemyCounter::GetUI()
{
	CTextRenderer* tr = GetTextRenderer();
	if (tr && tr->GetOwner())
	{
		return dynamic_cast<EnemyCount*>(tr->GetOwner());
	}
	return nullptr;
}

void EnemyCounter::RecountEnemies()
{
	int activeCount = 0;
	const auto& objectList = ObjectManager::GetInstance().GetObjectList();
	if (static_cast<size_t>(ObjectTag::ENEMY) < objectList.size())
	{
		for (const auto& obj : objectList[static_cast<size_t>(ObjectTag::ENEMY)])
		{
			if (obj && !obj->GetIsDestroyed())
			{
				activeCount++;
			}
		}
	}
	enemyCount_ = activeCount;
}

void EnemyCounter::ResetCount()
{
	defeatCount_ = 0;
	RecountEnemies();
	CTextRenderer* tr = GetTextRenderer();
	if (tr)
	{
		tr->SetText(L"Score : " + std::to_wstring(defeatCount_));
	}
}

void EnemyCounter::Init()
{
	CObject::Init();
	RecountEnemies();

	CTextRenderer* tr = GetTextRenderer();
	if (tr)
	{
		tr->SetText(L"Score : " + std::to_wstring(defeatCount_));
	}
}

// カウント
void EnemyCounter::Increment(int num_) { enemyCount_ += num_; OutputDebugStringA((std::to_string(enemyCount_) + "\n").c_str()); }

void EnemyCounter::Decrement(int num_) { enemyCount_ -= num_; if (enemyCount_ < 0) enemyCount_ = 0; OutputDebugStringA((std::to_string(enemyCount_) + "\n").c_str()); }

// 撃破
void EnemyCounter::Defeat(int num_)
{
	defeatCount_ += num_;

	// リアルタイムに敵の数を再計算
	RecountEnemies();

	CTextRenderer* tr = GetTextRenderer();
	if (tr)
	{
		tr->SetText(L"Score : " + std::to_wstring(defeatCount_));
	}

	// 敵が 0 以下ならクリアシーンに遷移
	if (enemyCount_ <= 0)
	{
		Event event;
		EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::Clear);

		event.SetEventData(eventData_NextScene);
		event.SetEventID(Events::ID::ChangeScene);

		EventManager::GetInstance().AddEvent(event);
	}
}
