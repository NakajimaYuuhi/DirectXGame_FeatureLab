//===== インクルード =====
#pragma once
#include "Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

//===== 前方宣言 =====
class CObject;

//===== クラス定義 =====
class SceneTitle : public CScene
{
public:
	SceneTitle();

	~SceneTitle();

	void Init();

	void Update();

	void Draw();


private:
};

