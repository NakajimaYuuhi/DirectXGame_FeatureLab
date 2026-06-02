
//===== インクルード =====
#pragma once
#include "3D_Object.h"

class Player : C3D_Object
{
public:
	Player(String _Name);

	//入力の更新
	virtual void Update();
};

