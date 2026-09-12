#include "SceneSerializer.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Model.h"
#include "Player.h"
#include "Enemy.h"
#include "Box.h"
#include "Source/External/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

bool SceneSerializer::SaveScene(const std::string& filepath, Scenes::ID sceneID)
{
	json rootJson;
	rootJson["sceneID"] = static_cast<int>(sceneID);
	rootJson["objects"] = json::array();

	const auto& objectList = ObjectManager::GetInstance().GetObjectList();

	for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
	{
		const auto& objVec = objectList[tagIdx];
		for (const auto& obj : objVec)
		{
			if (!obj || obj->GetIsDestroyed()) continue;

			CObjectInfo* objInfo = obj->GetComponent<CObjectInfo>();
			CTransform* transform = obj->GetComponent<CTransform>();

			if (!objInfo || !transform) continue;

			json objJson;
			objJson["name"] = objInfo->GetObjectName();
			objJson["tag"] = static_cast<int>(objInfo->GetObjectTag());

			DirectX::XMFLOAT3 pos = transform->GetPos();
			DirectX::XMFLOAT3 rot = transform->GetRotation();
			DirectX::XMFLOAT3 scale = transform->GetScale();

			objJson["transform"]["position"] = { pos.x, pos.y, pos.z };
			objJson["transform"]["rotation"] = { rot.x, rot.y, rot.z };
			objJson["transform"]["scale"] = { scale.x, scale.y, scale.z };

			rootJson["objects"].push_back(objJson);
		}
	}

	std::ofstream outFile(filepath);
	if (!outFile.is_open())
	{
		OutputDebugStringA(("[SceneSerializer] Failed to open file for writing: " + filepath + "\n").c_str());
		return false;
	}

	outFile << rootJson.dump(4);
	outFile.close();

	OutputDebugStringA(("[SceneSerializer] Successfully saved scene to: " + filepath + "\n").c_str());
	return true;
}

bool SceneSerializer::LoadScene(const std::string& filepath, Scenes::ID sceneID)
{
	std::ifstream inFile(filepath);
	if (!inFile.is_open())
	{
		OutputDebugStringA(("[SceneSerializer] Failed to open file for reading: " + filepath + "\n").c_str());
		return false;
	}

	json rootJson;
	try
	{
		inFile >> rootJson;
	}
	catch (const std::exception& e)
	{
		OutputDebugStringA(("[SceneSerializer] JSON Parse Error: " + std::string(e.what()) + "\n").c_str());
		return false;
	}
	inFile.close();

	if (!rootJson.contains("objects") || !rootJson["objects"].is_array())
	{
		return false;
	}

	// 既存オブジェクト�Eクリア
	auto& objectList = ObjectManager::GetInstance().GetObjectList();
	for (auto& vec : objectList)
	{
		for (auto& obj : vec)
		{
			if (obj) obj->SetIsDestroyed(true);
		}
	}
	ObjectManager::GetInstance().FlushDestroyedObjects();

	for (const auto& objJson : rootJson["objects"])
	{
		std::string name = objJson.value("name", "Object");
		int tagInt = objJson.value("tag", 0);
		ObjectTag tag = static_cast<ObjectTag>(tagInt);

		CObject* newObj = ObjectManager::GetInstance().Instantiate(sceneID, tag, name);

		if (newObj && objJson.contains("transform"))
		{
			CTransform* transform = newObj->GetComponent<CTransform>();
			if (transform)
			{
				const auto& transJson = objJson["transform"];
				if (transJson.contains("position") && transJson["position"].is_array() && transJson["position"].size() == 3)
				{
					transform->SetPos({ transJson["position"][0], transJson["position"][1], transJson["position"][2] });
				}
				if (transJson.contains("rotation") && transJson["rotation"].is_array() && transJson["rotation"].size() == 3)
				{
					transform->SetRotation({ transJson["rotation"][0], transJson["rotation"][1], transJson["rotation"][2] });
				}
				if (transJson.contains("scale") && transJson["scale"].is_array() && transJson["scale"].size() == 3)
				{
					transform->SetScale({ transJson["scale"][0], transJson["scale"][1], transJson["scale"][2] });
				}
			}

			newObj->Awake();
		}
	}

	OutputDebugStringA(("[SceneSerializer] Successfully loaded scene from: " + filepath + "\n").c_str());
	return true;
}