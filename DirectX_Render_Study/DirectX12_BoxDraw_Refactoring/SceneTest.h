//===== インクルード =====
#pragma once
#include "Scene.h"

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
	Vector<UniquePtr<CObject>> m_VecObject;
	void AddObject(UniquePtr<CObject> _Object);
};

