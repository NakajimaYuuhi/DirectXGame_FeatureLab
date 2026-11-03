#pragma once
#include "EventData.h"
#include "SceneEnums.h"
#include <string>

class EventData_NextScene : public EventData
{
public:
	// コンストラクタ (旧Scenes::ID互換)
	EventData_NextScene(Scenes::ID _NextScene)
		: nextScene(_NextScene)
	{}

	// コンストラクタ (データ駆動シーンパス指定)
	EventData_NextScene(const std::string& _NextScenePath, Scenes::ID _FallbackID = Scenes::ID::NONE)
		: nextScene(_FallbackID), nextScenePath(_NextScenePath)
	{}

	// デストラクタ
	virtual ~EventData_NextScene() = default;

	// 次のシーン取得
	Scenes::ID GetNextScene(void) const { return nextScene; }
	const std::string& GetNextScenePath(void) const { return nextScenePath; }
	bool HasScenePath(void) const { return !nextScenePath.empty(); }

private:
	Scenes::ID nextScene = Scenes::ID::NONE;
	std::string nextScenePath;
};
