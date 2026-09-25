
//ヘッダ
#include "EnemyCounter.h"

#include "ObjectManager.h"

#include "EnemyCount.h"

// イベント管理
#include "EventManager.h"

#include "EventData_NextScene.h"


//コンストラクタ
EnemyCounter::EnemyCounter(String _Name) 
	:enemyCount_	(0) 
	,defeatCount_	(0)
	,enemyCountUI_  (nullptr)
{ 
	SetName(_Name); 
}

EnemyCount* EnemyCounter::GetUI()
{
	if (!enemyCountUI_)
	{
		const auto& objectList = ObjectManager::GetInstance().GetObjectList();
		if (static_cast<size_t>(ObjectTag::TEXT) < objectList.size())
		{
			for (const auto& obj : objectList[static_cast<size_t>(ObjectTag::TEXT)])
			{
				if (!obj || obj->GetIsDestroyed()) continue;
				EnemyCount* textUI = dynamic_cast<EnemyCount*>(obj.get());
				if (textUI)
				{
					enemyCountUI_ = textUI;
					break;
				}
			}
		}
	}
	return enemyCountUI_;
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
	EnemyCount* ui = GetUI();
	if (ui)
	{
		ui->UpdateText(defeatCount_);
	}
}

void EnemyCounter::Init()
{
	CObject::Init();

	EnemyCount* ui = GetUI();
	if (!ui)
	{
		enemyCountUI_ = (EnemyCount*)ObjectManager::GetInstance().Instantiate(Scenes::NONE, ObjectTag::TEXT, "EnemyCount", "EnemyCount");
		ui = enemyCountUI_;
	}

	RecountEnemies();

	if (ui)
	{
		ui->UpdateText(defeatCount_);
	}
}

//カウント
void EnemyCounter::Increment(int num_) { enemyCount_ += num_; OutputDebugStringA((std::to_string(enemyCount_) + "\n").c_str()); }

void EnemyCounter::Decrement(int num_) { enemyCount_ -= num_; if (enemyCount_ < 0) enemyCount_ = 0; OutputDebugStringA((std::to_string(enemyCount_) + "\n").c_str()); }

//撃破時
void EnemyCounter::Defeat(int num_)
{
	defeatCount_ += num_;

	// リアルタイムに生存敵の数を再計算
	RecountEnemies();

	EnemyCount* ui = GetUI();
	if (ui)
	{
		ui->UpdateText(defeatCount_);
	}

	// 生存敵が 0 以下ならクリアシーンに遷移
	if (enemyCount_ <= 0)
	{
		Event event;
		EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::Clear);

		event.SetEventData(eventData_NextScene);
		event.SetEventID(Events::ID::ChangeScene);

		EventManager::GetInstance().AddEvent(event);
	}
}
