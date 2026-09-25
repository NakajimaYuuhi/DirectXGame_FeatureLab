#pragma once
#include "EventData.h"
#include "SceneEnums.h"

class EventData_NextScene : public EventData
{
public:
	//?R???X?g???N?^
	EventData_NextScene(Scenes::ID _NextScene)
		:nextScene(_NextScene)
	{}

	//?f?X?g???N?^
	virtual ~EventData_NextScene() = default;

	//????V?[????èÔ
	Scenes::ID GetNextScene(void) const { return nextScene; }

private:
	//????V?[??
	Scenes::ID nextScene;
};

