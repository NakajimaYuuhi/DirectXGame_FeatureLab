//敵の数を表示する

#pragma once
#include "TextObject.h"
#include "StringAlias.h"
class EnemyCount :public TextObject
{


public:
	EnemyCount(String _Name);
	virtual ~EnemyCount() = default;

	//テキストの更新
	void UpdateText(int num_);

private:
	wstring BeginText = L"Score : ";

};

