#pragma once
#include "SceneEnums.h"
#include <string>

class SceneSerializer
{
public:
	static bool SaveScene(const std::string& filepath, Scenes::ID sceneID);
	static bool LoadScene(const std::string& filepath, Scenes::ID sceneID);
	static bool LoadSceneOrDefault(const std::string& filepath, Scenes::ID sceneID);
};
