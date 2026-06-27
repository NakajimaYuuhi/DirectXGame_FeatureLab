#pragma once
#include "Manager.h"
#include <Windows.h>
#include "StringAlias.h"

class EnemyCount;

class EnemyCounter :public Manager
{
public:
	EnemyCounter(String _Name);
	~EnemyCounter() = default;

	virtual void Init();
	

	//カウント処理
	void Increment(int num_ = 1);
	void Decrement(int num_ = 1);
	void ResetCount() { enemyCount_; }


	//生成した
	void Instantiated(int num_ = 1)
	{
		Increment(num_);
	}

	//撃破
	void Defeat(int num_ = 1);

protected:
	//今の所は、数だけ
	int enemyCount_;
	int defeatCount_;
	EnemyCount* enemyCountUI_;



	//----- Getter -----
public:
	int GetCount()			{ return enemyCount_; }
	int GetDefeatCount()	{ return defeatCount_; }
};

