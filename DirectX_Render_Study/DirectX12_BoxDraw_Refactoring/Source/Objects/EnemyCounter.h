#pragma once
#include "Manager.h"
#include <Windows.h>
#include "StringAlias.h"

class EnemyCount;
class CTextRenderer;

class EnemyCounter : public Manager
{
public:
	EnemyCounter(String _Name);
	~EnemyCounter() = default;

	virtual void Init() override;
	

	//ƒJƒEƒ“ƒg
	void Increment(int num_ = 1);
	void Decrement(int num_ = 1);
	void ResetCount();
	void RecountEnemies();


	//¶¬
	void Instantiated(int num_ = 1)
	{
		RecountEnemies();
	}

	//Œ‚”j
	void Defeat(int num_ = 1);

	EnemyCount* GetUI();
	CTextRenderer* GetTextRenderer();

protected:
	int enemyCount_;
	int defeatCount_;
	EnemyCount* enemyCountUI_ = nullptr;
	CTextRenderer* m_textRenderer = nullptr;



	//----- Getter -----
public:
	int GetCount()			{ return enemyCount_; }
	int GetDefeatCount()	{ return defeatCount_; }
};

