#include "SceneSerializer.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Model.h"
#include "Player.h"
#include "Enemy.h"
#include "Box.h"
#include "UIObject.h"
#include "CUIButton.h"
#include "TextObject.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"
#include "ButtonEventManager.h"
#include "ButtonAction.h"
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
			CTransform* transform = obj->GetComponent<CTransform>();

			if (!objInfo || !transform) continue;

			json objJson;
			objJson["name"] = objInfo->GetObjectName();
			objJson["tag"] = static_cast<int>(objInfo->GetObjectTag());

			if (dynamic_cast<CUIButton*>(obj.get()))
				objJson["type"] = "CUIButton";
			else if (dynamic_cast<CUIObject*>(obj.get()))
				objJson["type"] = "CUIObject";
			else if (dynamic_cast<TextObject*>(obj.get()))
				objJson["type"] = "TextObject";
			else if (dynamic_cast<Player*>(obj.get()))
				objJson["type"] = "Player";
			else if (dynamic_cast<Enemy*>(obj.get()))
				objJson["type"] = "Enemy";
			else
				objJson["type"] = "CObject";

			DirectX::XMFLOAT3 pos = transform->GetPos();
			DirectX::XMFLOAT3 rot = transform->GetRotation();
			DirectX::XMFLOAT3 scale = transform->GetScale();

			objJson["transform"]["position"] = { pos.x, pos.y, pos.z };
			objJson["transform"]["rotation"] = { rot.x, rot.y, rot.z };
			objJson["transform"]["scale"] = { scale.x, scale.y, scale.z };

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

			// CUIButton
			CUIButton* btn = dynamic_cast<CUIButton*>(obj.get());
			if (btn)
			{
				objJson["button"]["action"] = ButtonActionToString(btn->GetAction());
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

	// Clear objects
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

			// CUIButton
			CUIButton* btn = dynamic_cast<CUIButton*>(newObj);
			if (btn && objJson.contains("button"))
			{
				const auto& btnJson = objJson["button"];
				if (btnJson.contains("action"))
				{
					btn->SetAction(StringToButtonAction(btnJson["action"].get<std::string>()));
				}
			}

			newObj->Awake();
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

