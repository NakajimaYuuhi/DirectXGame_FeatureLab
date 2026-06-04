#pragma once
#include "BillBoard.h"
class RandomParticle : public BillBoard
{
public:
	RandomParticle(String _Name);
	virtual void Update() override;
protected:
	//•ûŒü
	DirectX::XMFLOAT3 Direction;
	//‘¬“x
	float Speed = 0.01f;
	//Žõ–½
	int LifeTime = 120;

};
