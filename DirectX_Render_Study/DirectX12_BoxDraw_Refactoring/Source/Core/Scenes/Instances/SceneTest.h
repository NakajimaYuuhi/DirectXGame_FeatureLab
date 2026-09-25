//===== ?C???N???[?h =====
#pragma once
#include "../Base/Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

//===== ?O???? =====
class CObject;

//===== ?N???X??` =====
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

