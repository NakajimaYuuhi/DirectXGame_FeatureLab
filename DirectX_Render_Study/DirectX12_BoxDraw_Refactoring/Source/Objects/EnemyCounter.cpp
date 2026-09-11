
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
{ 
	SetName(_Name); 
}


void EnemyCounter::Init()
{
	CObject::Init();

	//UIの生成
	enemyCountUI_ = (EnemyCount*)ObjectManager::GetInstance().Instantiate(Scenes::NONE, ObjectTag::TEXT, "EnemyCount");
	enemyCountUI_->UpdateText(defeatCount_);
}

//カウント処理
void EnemyCounter::Increment(int num_) { enemyCount_ += num_; OutputDebugString(std::to_string(enemyCount_).c_str()); }

void EnemyCounter::Decrement(int num_) { enemyCount_ -= num_; OutputDebugString(std::to_string(enemyCount_).c_str()); }

//撃破
void EnemyCounter::Defeat(int num_)
{
	defeatCount_ += num_;
	enemyCountUI_->UpdateText(defeatCount_);
	Decrement(num_);

	// 撃破したうえでEnemyカウントが0になったら、クリアシーンに遷移
	if (enemyCount_ <= 0)
	{
		// クリアシーンに遷移
		Event event;
		EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::Clear);

		event.SetEventData(eventData_NextScene);

		event.SetEventID(Events::ID::ChangeScene);

		EventManager::GetInstance().AddEvent(event);
	}
}
