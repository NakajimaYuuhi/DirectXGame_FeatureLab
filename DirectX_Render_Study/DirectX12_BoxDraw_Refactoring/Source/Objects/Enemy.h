#pragma once
#include "3D_Object.h"
class Enemy :
    public C3D_Object
{
public:
	Enemy(String _Name);
	~Enemy() = default;

	//“ü—Í‚ÌXV

	virtual void Init();
	virtual void Update();

	virtual void OnCollision(CObject* _Other) override;

protected:
	String ModelPath;

	float Speed = 0.005f;

	int HP = 3;
};

