#pragma once
#include "EventData.h"
#include "SceneEnums.h"

class EventData_NextScene : public EventData
{
public:
	//コンストラクタ
	EventData_NextScene(Scenes::ID _NextScene)
		:nextScene(_NextScene)
	{}

	//デストラクタ
	virtual ~EventData_NextScene() = default;

	//次のシーンの取得
	Scenes::ID GetNextScene(void) const { return nextScene; }

private:
	//次のシーン
	Scenes::ID nextScene;
};

