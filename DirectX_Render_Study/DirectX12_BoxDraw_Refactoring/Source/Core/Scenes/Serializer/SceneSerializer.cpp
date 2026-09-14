#include "SceneSerializer.h"
#include "ObjectManager.h"
#include "ObjectInfo.h"
#include "Transform.h"
#include "Model.h"
#include "Player.h"
#include "Enemy.h"
#include "Skydome.h"
#include "EnemyCounter.h"
#include "EnemyCount.h"
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
			if (!objInfo) continue;

			json objJson;
			objJson["name"] = objInfo->GetObjectName();
			objJson["tag"] = static_cast<int>(objInfo->GetObjectTag());

			if (dynamic_cast<CUIButton*>(obj.get()))
				objJson["type"] = "CUIButton";
			else if (dynamic_cast<CUIObject*>(obj.get()))
				objJson["type"] = "CUIObject";
			else if (dynamic_cast<EnemyCount*>(obj.get()))
				objJson["type"] = "EnemyCount";
			else if (dynamic_cast<TextObject*>(obj.get()))
				objJson["type"] = "TextObject";
			else if (dynamic_cast<Player*>(obj.get()))
				objJson["type"] = "Player";
			else if (dynamic_cast<Enemy*>(obj.get()))
				objJson["type"] = "Enemy";
			else if (dynamic_cast<Skydome*>(obj.get()))
				objJson["type"] = "Skydome";
			else if (dynamic_cast<EnemyCounter*>(obj.get()))
				objJson["type"] = "EnemyCounter";
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

			// CUIButton
			CUIButton* btn = dynamic_cast<CUIButton*>(obj.get());
			if (btn)
			{
				objJson["button"]["action"] = ButtonActionToString(btn->GetAction());

				auto GetNavName = [](CUIButton* targetBtn) -> std::string {
					if (!targetBtn) return "";
					CObjectInfo* info = targetBtn->GetComponent<CObjectInfo>();
					return info ? info->GetObjectName() : "";
				};

				objJson["button"]["navigation"]["up"] = GetNavName(btn->GetSelectOnUp());
				objJson["button"]["navigation"]["down"] = GetNavName(btn->GetSelectOnDown());
				objJson["button"]["navigation"]["left"] = GetNavName(btn->GetSelectOnLeft());
				objJson["button"]["navigation"]["right"] = GetNavName(btn->GetSelectOnRight());
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

	struct PendingNav
	{
		CUIButton* btn;
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

		if (tag == ObjectTag::NONE || tagInt == -1)
		{
			if (type == "CUIObject" || type == "CUIButton")
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
			else if (type == "EnemyCounter")
			{
				tag = ObjectTag::MANAGER;
			}
		}

		CObject* newObj = ObjectManager::GetInstance().Instantiate(sceneID, tag, type.empty() ? name : type, name);

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
				if (btnJson.contains("navigation"))
				{
					const auto& navJson = btnJson["navigation"];
					PendingNav pnav;
					pnav.btn = btn;
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

	auto FindButtonByName = [](const std::string& targetName) -> CUIButton* {
		if (targetName.empty()) return nullptr;
		const auto& objectList = ObjectManager::GetInstance().GetObjectList();
		for (const auto& vec : objectList)
		{
			for (const auto& obj : vec)
			{
				if (!obj || obj->GetIsDestroyed()) continue;
				CObjectInfo* info = obj->GetComponent<CObjectInfo>();
				if (info && info->GetObjectName() == targetName)
				{
					return dynamic_cast<CUIButton*>(obj.get());
				}
			}
		}
		return nullptr;
	};

	if (!pendingNavs.empty())
	{
		for (const auto& pnav : pendingNavs)
		{
			CUIButton* upBtn = FindButtonByName(pnav.up);
			CUIButton* downBtn = FindButtonByName(pnav.down);
			CUIButton* leftBtn = FindButtonByName(pnav.left);
			CUIButton* rightBtn = FindButtonByName(pnav.right);
			pnav.btn->SetNavigation(upBtn, downBtn, leftBtn, rightBtn);
		}
	}
	else
	{
		// Auto-link navigation for loaded CUIButtons if no explicit navigation was present
		std::vector<CUIButton*> loadedButtons;
		const auto& loadedObjectList = ObjectManager::GetInstance().GetObjectList();
		for (const auto& vec : loadedObjectList)
		{
			for (const auto& obj : vec)
			{
				if (!obj || obj->GetIsDestroyed()) continue;
				CUIButton* btn = dynamic_cast<CUIButton*>(obj.get());
				if (btn)
				{
					loadedButtons.push_back(btn);
				}
			}
		}

		if (loadedButtons.size() > 1)
		{
			for (size_t i = 0; i < loadedButtons.size(); ++i)
			{
				CUIButton* prev = loadedButtons[(i + loadedButtons.size() - 1) % loadedButtons.size()];
				CUIButton* next = loadedButtons[(i + 1) % loadedButtons.size()];
				loadedButtons[i]->SetNavigation(prev, next, nullptr, nullptr);
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


