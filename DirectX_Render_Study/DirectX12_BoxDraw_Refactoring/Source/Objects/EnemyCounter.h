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

	virtual void Init() override;
	

	//カウント
	void Increment(int num_ = 1);
	void Decrement(int num_ = 1);
	void ResetCount();
	void RecountEnemies();


	//生成時
	void Instantiated(int num_ = 1)
	{
		Increment(num_);
	}

	//撃破時
	void Defeat(int num_ = 1);

	EnemyCount* GetUI();

protected:
	//敵の情報は、ここで
	int enemyCount_;
	int defeatCount_;
	EnemyCount* enemyCountUI_ = nullptr;



	//----- Getter -----
public:
	int GetCount()			{ return enemyCount_; }
	int GetDefeatCount()	{ return defeatCount_; }
};

