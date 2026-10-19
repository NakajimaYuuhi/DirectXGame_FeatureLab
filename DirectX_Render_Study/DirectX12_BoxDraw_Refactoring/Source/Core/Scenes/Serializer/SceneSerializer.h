#pragma once
#include "SceneEnums.h"
#include <string>

class SceneSerializer
{
public:
	static bool SaveScene(const std::string& filepath, Scenes::ID sceneID);
	static bool LoadScene(const std::string& filepath, Scenes::ID sceneID);
	static bool LoadSceneOrDefault(const std::string& filepath, Scenes::ID sceneID);

	// In-memory snapshot serialization / deserialization (used by Play/Stop mode and C++ DLL Hot Reload)
	static bool SerializeToString(std::string& outJsonStr, Scenes::ID sceneID);
	static bool DeserializeFromString(const std::string& jsonStr, Scenes::ID sceneID);
};
