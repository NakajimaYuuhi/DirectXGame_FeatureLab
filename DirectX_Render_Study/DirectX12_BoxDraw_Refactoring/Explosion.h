#pragma once
#include "BillBoard.h"
class Explosion :
    public BillBoard
{
public:
	Explosion(String _Name);
	virtual void Update() override;
protected:

	//寿命
	int LifeTime = 120;

	//アニメーション番号
	int Frame = 0;

	//フレーム数
	int MaxFrame = 16;

	//更新間隔
	int UpdateInterval = 6;
};

