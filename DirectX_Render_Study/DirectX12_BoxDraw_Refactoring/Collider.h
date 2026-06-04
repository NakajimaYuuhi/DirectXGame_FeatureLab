#pragma once
#include "Component.h"
class Collider : public CComponent
{
public:
	typedef enum
	{
		//NONE,//デフォルト用
		CIRCLE_2D,
		CIRCLE_2D_Trigger,
		CIRCLE_3D,
		CIRCLE_3D_Trigger,
		BOX_2D,
		BOX_2D_Trigger,
		BOX_3D,
		BOX_3D_Trigger,

	}CololiderType;



	//コンストラクタ
	Collider() {}

	//デストラクタ
	virtual ~Collider(){}

	//Update Transformに従属してUpdate
	//Getter
	virtual CololiderType GetColliderType() { return ColliderType; }
	bool GetIsTrigger() { return IsTrriger; }


	bool IsTrriger = false;//デフォルトはfalse

protected:
	CololiderType ColliderType;

private:
};

