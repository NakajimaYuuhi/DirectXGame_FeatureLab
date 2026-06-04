
//===== インクルード =====
#pragma once
#include "3D_Object.h"

#include "StringAlias.h"

class Player : public C3D_Object
{
public:
	Player(String _Name);
	~Player() = default;

	//入力の更新
	virtual void Update();

protected:
	String ModelPath;

	float Speed = 0.01;
};

