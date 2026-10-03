#include "SceneSerializer.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Model.h"
#include "Field.h"
#include "Box.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "ButtonEventManager.h"
#include "ButtonAction.h"
#include "Camera.h"
#include "CameraComponent.h"
#include "ButtonComponent.h"
#include "PrefabManager.h"
#include "Source/External/json.hpp"
#include <fstream>
#include <iostream>
#include <windows.h>

using json = nlohmann::json;

static std::string WStringToString(const std::wstring& wstr)
{
	if (wstr.empty()) return "";
	int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
	std::string str(size, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size, NULL, NULL);
	return str;
}

static std::wstring StringToWString(const std::string& str)
{
	if (str.empty()) return L"";
	int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstr[0], size);
	return wstr;
}

bool SceneSerializer::SaveScene(const std::string& filepath, Scenes::ID sceneID)
{
	ObjectManager::GetInstance().FlushDestroyedObjects();

	json rootJson;
	rootJson["sceneID"] = static_cast<int>(sceneID);
	rootJson["eventSystem"]["firstSelected"] = ButtonEventManager::GetInstance().GetFirstSelectedName();
	rootJson["objects"] = json::array();

	const auto& objectList = ObjectManager::GetInstance().GetObjectList();

	for (size_t tagIdx = 0; tagIdx < objectList.size(); ++tagIdx)
	{
		const auto& objVec = objectList[tagIdx];
		for (const auto& obj : objVec)
		{
			if (!obj || obj->GetIsDestroyed()) continue;

			CObjectInfo* objInfo = obj->GetComponent<CObjectInfo>();
			if (!objInfo) continue;

			json objJson;
			objJson["name"] = objInfo->GetObjectName();
			objJson["tag"] = static_cast<int>(objInfo->GetObjectTag());
			if (objInfo->IsPrefab())
			{
				objJson["prefab"] = objInfo->GetPrefabName();
			}

			if (objInfo->GetObjectTag() == ObjectTag::PLAYER)
				objJson["type"] = "Player";
			else if (objInfo->GetObjectTag() == ObjectTag::ENEMY)
				objJson["type"] = "Enemy";
			else if (objInfo->GetObjectTag() == ObjectTag::BACKGROUND)
				objJson["type"] = "Skydome";
			else if (dynamic_cast<Field*>(obj.get()))
				objJson["type"] = "Field";
			else if (objInfo->GetObjectTag() == ObjectTag::CAMERA || obj->GetComponent<CameraComponent>() || dynamic_cast<Camera*>(obj.get()))
				objJson["type"] = "Camera";
			else
				objJson["type"] = "CObject";

			CTransform* transform = obj->GetComponent<CTransform>();
			if (transform)
			{
				DirectX::XMFLOAT3 pos = transform->GetPos();
				DirectX::XMFLOAT3 rot = transform->GetRotation();
				DirectX::XMFLOAT3 scale = transform->GetScale();

				objJson["transform"]["position"] = { pos.x, pos.y, pos.z };
				objJson["transform"]["rotation"] = { rot.x, rot.y, rot.z };
				objJson["transform"]["scale"] = { scale.x, scale.y, scale.z };
			}

			// CSpriteRenderer
			CSpriteRenderer* sprite = obj->GetComponent<CSpriteRenderer>();
			if (sprite)
			{
				objJson["sprite"]["texturePath"] = WStringToString(sprite->GetTexturePath());
				DirectX::XMFLOAT2 sz = sprite->GetSize();
				objJson["sprite"]["size"] = { sz.x, sz.y };
				DirectX::XMFLOAT4 col = sprite->GetColor();
				objJson["sprite"]["color"] = { col.x, col.y, col.z, col.w };
			}

			// CTextRenderer
			CTextRenderer* textComp = obj->GetComponent<CTextRenderer>();
			if (textComp)
			{
				objJson["text"]["content"] = WStringToString(textComp->GetText());
				objJson["text"]["position"] = { transform->GetPos().x, transform->GetPos().y };
				objJson["text"]["fontSize"] = textComp->GetFontSize();
				D2D1::ColorF col = textComp->GetColor();
				objJson["text"]["color"] = { col.r, col.g, col.b, col.a };
				objJson["text"]["fontFamily"] = WStringToString(textComp->GetFontFamily());
			}

			

			// ButtonComponent
			ButtonComponent* btnComp = obj->GetComponent<ButtonComponent>();
			if (btnComp)
			{
				objJson["button"]["action"] = ButtonActionToString(btnComp->GetAction());
				objJson["button"]["navigation"]["up"] = btnComp->GetUpName();
				objJson["button"]["navigation"]["down"] = btnComp->GetDownName();
				objJson["button"]["navigation"]["left"] = btnComp->GetLeftName();
				objJson["button"]["navigation"]["right"] = btnComp->GetRightName();
			}

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

	if (rootJson.contains("eventSystem") && rootJson["eventSystem"].contains("firstSelected"))
	{
		std::string firstSel = rootJson["eventSystem"].value("firstSelected", "");
		ButtonEventManager::GetInstance().SetFirstSelectedName(firstSel);
	}

	// Clear objects completely including pending additions
	ObjectManager::GetInstance().Uninit();

	struct PendingNav
	{
		CObject* obj;
		std::string up;
		std::string down;
		std::string left;
		std::string right;
	};
	std::vector<PendingNav> pendingNavs;

	for (const auto& objJson : rootJson["objects"])
	{
		std::string name = objJson.value("name", "Object");
		int tagInt = objJson.value("tag", static_cast<int>(ObjectTag::NONE));
		ObjectTag tag = static_cast<ObjectTag>(tagInt);
		std::string type = objJson.value("type", "");

		if (tag == ObjectTag::NONE || tagInt == -1 || type == "Camera" || name == "Camera" || type == "CameraComponent")
		{
			if (type == "Camera" || name == "Camera" || type == "CameraComponent")
			{
				tag = ObjectTag::CAMERA;
			}
			else if (type == "CUIObject" || type == "CUIButton" || type == "UIImage" || type == "UIButton")
			{
				tag = ObjectTag::UI;
			}
			else if (type == "TextObject" || type == "EnemyCount")
			{
				tag = ObjectTag::TEXT;
			}
			else if (type == "Player")
			{
				tag = ObjectTag::PLAYER;
			}
			else if (type == "Enemy")
			{
				tag = ObjectTag::ENEMY;
			}
			else if (type == "Skydome")
			{
				tag = ObjectTag::BACKGROUND;
			}
			else if (type == "Field")
			{
				tag = ObjectTag::FIELD;
			}
			else if (type == "EnemyCounter")
			{
				tag = ObjectTag::MANAGER;
			}
		}

		std::string prefabStr = objJson.value("prefab", "");
		std::string instantiateType = (!prefabStr.empty() && PrefabManager::GetInstance().HasPrefab(prefabStr))
			? prefabStr
			: (type.empty() ? name : type);

		CObject* newObj = ObjectManager::GetInstance().Instantiate(sceneID, tag, instantiateType, name);
		if (newObj)
		{
			if (auto info = newObj->GetComponent<CObjectInfo>())
			{
				if (!prefabStr.empty())
				{
					info->SetPrefabName(prefabStr);
				}
				if (type == "Camera" || name == "Camera" || type == "CameraComponent")
				{
					info->SetObjectTag(ObjectTag::CAMERA);
				}
			}
		}

		if (newObj)
		{
			if (objJson.contains("transform"))
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
			}

			// CSpriteRenderer
			CSpriteRenderer* sprite = newObj->GetComponent<CSpriteRenderer>();
			if (sprite && objJson.contains("sprite"))
			{
				const auto& spriteJson = objJson["sprite"];
				if (spriteJson.contains("texturePath"))
				{
					std::string texPathStr = spriteJson["texturePath"].get<std::string>();
					if (!texPathStr.empty())
					{
						sprite->SetTexture(StringToWString(texPathStr));
					}
				}
				if (spriteJson.contains("size") && spriteJson["size"].is_array() && spriteJson["size"].size() == 2)
				{
					sprite->SetSize(spriteJson["size"][0], spriteJson["size"][1]);
				}
				if (spriteJson.contains("color") && spriteJson["color"].is_array() && spriteJson["color"].size() == 4)
				{
					sprite->SetColor({ spriteJson["color"][0], spriteJson["color"][1], spriteJson["color"][2], spriteJson["color"][3] });
				}
			}

			// CTextRenderer
			CTextRenderer* textComp = newObj->GetComponent<CTextRenderer>();
			if (!textComp && objJson.contains("text"))
			{
				textComp = newObj->AddComponent<CTextRenderer>();
			}
			if (textComp && objJson.contains("text"))
			{
				const auto& textJson = objJson["text"];
				if (textJson.contains("content"))
				{
					textComp->SetText(StringToWString(textJson["content"].get<std::string>()));
				}
				if (textJson.contains("position") && textJson["position"].is_array() && textJson["position"].size() == 2)
				{
					float tx = textJson["position"][0];
					float ty = textJson["position"][1];
					CTransform* transform = newObj->GetComponent<CTransform>();
					if (transform)
					{
						transform->SetPos({ tx, ty, 0.0f });
					}
					textComp->SetPosition(0.0f, 0.0f);
				}
				if (textJson.contains("fontSize"))
				{
					textComp->SetFontSize(textJson["fontSize"].get<float>());
				}
				if (textJson.contains("color") && textJson["color"].is_array() && textJson["color"].size() == 4)
				{
					const auto& c = textJson["color"];
					textComp->SetColor(D2D1::ColorF(c[0], c[1], c[2], c[3]));
				}
				if (textJson.contains("fontFamily"))
				{
					textComp->SetFontFamily(StringToWString(textJson["fontFamily"].get<std::string>()));
				}
			}

			// Button (CUIButton or ButtonComponent)
			if (objJson.contains("button") || objJson.contains("buttonComponent"))
			{
				const auto& btnJson = objJson.contains("button") ? objJson["button"] : objJson["buttonComponent"];
				ButtonAction action = ButtonAction::None;
				bool hasAction = false;
				if (btnJson.contains("action"))
				{
					action = StringToButtonAction(btnJson["action"].get<std::string>());
					hasAction = true;
				}

				ButtonComponent* btnComp = newObj->GetComponent<ButtonComponent>();
				if (!btnComp)
				{
					btnComp = newObj->AddComponent<ButtonComponent>();
				}
				if (btnComp && hasAction)
				{
					btnComp->SetAction(action);
				}

				if (btnJson.contains("navigation"))
				{
					const auto& navJson = btnJson["navigation"];
					PendingNav pnav;
					pnav.obj = newObj;
					pnav.up = navJson.value("up", "");
					pnav.down = navJson.value("down", "");
					pnav.left = navJson.value("left", "");
					pnav.right = navJson.value("right", "");
					pendingNavs.push_back(pnav);
				}
			}

			newObj->Init();
			newObj->Awake();
		}
	}

	// Flush all instantiated objects so that navigation links and ApplyFirstSelected can locate them
	ObjectManager::GetInstance().FlushPendingAddObjects();

	if (!pendingNavs.empty())
	{
		for (const auto& pnav : pendingNavs)
		{
			if (!pnav.obj) continue;

			ButtonComponent* bComp = pnav.obj->GetComponent<ButtonComponent>();
			if (bComp)
			{
				bComp->SetNavigationNames(pnav.up, pnav.down, pnav.left, pnav.right);
			}
		}
	}

	ButtonEventManager::GetInstance().ApplyFirstSelected();



	OutputDebugStringA(("[SceneSerializer] Successfully loaded scene from: " + filepath + "\n").c_str());
	return true;
}

bool SceneSerializer::LoadSceneOrDefault(const std::string& filepath, Scenes::ID sceneID)
{
	std::ifstream inFile(filepath);
	if (inFile.is_open())
	{
		inFile.close();
		return LoadScene(filepath, sceneID);
	}
	return false;
}


