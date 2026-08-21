
//ヘッダ
#include "EnemyCounter.h"

#include "ObjectManager.h"

#include "EnemyCount.h"


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
}
