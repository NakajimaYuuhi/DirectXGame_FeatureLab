//===== インクルード =====
#pragma once
#include "../Base/Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

//===== 前方宣言 =====
class CObject;

//===== クラス定義 =====
class CSceneTest : public CScene
{
public:
	CSceneTest();

	~CSceneTest();

	void Init();

	void Update();

	void Draw();


private:
};

