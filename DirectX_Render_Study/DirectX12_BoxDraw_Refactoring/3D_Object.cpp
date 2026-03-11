//===== インクルード =====
#include "3D_Object.h"

//Transform
#include "Transform.h"

C3D_Object::C3D_Object()
	:CObject()
{
	//Transformコンポーネントの作成
	AddComponent<CTransform>();

	//Meshもここ？
}
