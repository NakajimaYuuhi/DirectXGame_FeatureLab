#include "SceneManager.h"
#include <fstream>
#include "ObjectManager.h"
#include "../Base/Scene.h"
#include "SceneFade.h"
#include "ButtonEventManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Event.h"
#include "EventData_NextScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "DX12Manager.h"
#include "InspectorUI.h"
#include "Source/External/json.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

static std::string NormalizeScenePath(const std::string& rawPath)
{
	if (rawPath.empty()) return "";
	std::string p = rawPath;
	std::replace(p.begin(), p.end(), '\\', '/');
	if (p.find('/') == std::string::npos)
	{
		p = "Assets/Scene/" + p;
	}
	if (fs::path(p).extension().empty())
	{
		p += ".json";
	}
	return p;
}

static std::string NormalizeFolderPath(const std::string& rawPath)
{
	if (rawPath.empty()) return "";
	std::string p = rawPath;
	std::replace(p.begin(), p.end(), '\\', '/');
	while (!p.empty() && p.back() == '/')
	{
		p.pop_back();
	}
	return p;
}

static std::unordered_map<int, std::string> s_sceneIdToPath;
static std::unordered_map<std::string, Scenes::ID> s_pathToSceneId;
static bool s_registryInitialized = false;

void SceneManager::RefreshSceneRegistry()
{
	s_sceneIdToPath.clear();
	s_pathToSceneId.clear();
	s_registryInitialized = true;

	std::error_code ec;
	fs::path sceneDir("Assets/Scene");
	if (fs::exists(sceneDir, ec) && fs::is_directory(sceneDir, ec))
	{
		for (const auto& entry : fs::recursive_directory_iterator(sceneDir, ec))
		{
			if (!entry.is_regular_file(ec)) continue;
			auto ext = entry.path().extension();
			if (ext != ".json" && ext != ".scene") continue;
			std::string normPath = entry.path().generic_string();

			std::ifstream file(normPath);
			if (file.is_open())
			{
				nlohmann::json j;
				try
				{
					file >> j;
					if (j.contains("sceneID") && j["sceneID"].is_number())
					{
						int idInt = j["sceneID"].get<int>();
						Scenes::ID sId = static_cast<Scenes::ID>(idInt);
						s_sceneIdToPath[idInt] = normPath;
						s_pathToSceneId[normPath] = sId;
					}
				}
				catch (...) {}
			}
		}
	}
}

std::string SceneManager::ResolveScenePathFromID(Scenes::ID sceneID)
{
	if (!s_registryInitialized)
	{
		RefreshSceneRegistry();
	}
	int idInt = static_cast<int>(sceneID);
	auto it = s_sceneIdToPath.find(idInt);
	if (it != s_sceneIdToPath.end() && !it->second.empty())
	{
		return it->second;
	}

	switch (sceneID)
	{
	case Scenes::ID::TITLE:  return "Assets/Scene/SceneTitle.json";
	case Scenes::ID::TEST:   return "Assets/Scene/SceneTest.json";
	case Scenes::ID::Clear:  return "Assets/Scene/SceneClear.json";
	case Scenes::ID::Failed: return "Assets/Scene/SceneFailed.json";
	default:                 return "Assets/Scene/SceneTest.json";
	}
}

Scenes::ID SceneManager::ResolveSceneIDFromPath(const std::string& jsonPath)
{
	if (!s_registryInitialized)
	{
		RefreshSceneRegistry();
	}
	std::string normPath = NormalizeScenePath(jsonPath);
	auto it = s_pathToSceneId.find(normPath);
	if (it != s_pathToSceneId.end())
	{
		return it->second;
	}

	std::ifstream file(normPath);
	if (file.is_open())
	{
		nlohmann::json j;
		try
		{
			file >> j;
			if (j.contains("sceneID") && j["sceneID"].is_number())
			{
				Scenes::ID sId = static_cast<Scenes::ID>(j["sceneID"].get<int>());
				s_pathToSceneId[normPath] = sId;
				s_sceneIdToPath[static_cast<int>(sId)] = normPath;
				return sId;
			}
		}
		catch (...) {}
	}

	std::string stem = fs::path(normPath).stem().string();
	if (stem == "SceneTitle")  return Scenes::ID::TITLE;
	if (stem == "SceneClear")  return Scenes::ID::Clear;
	if (stem == "SceneFailed") return Scenes::ID::Failed;
	if (stem == "SceneTest")   return Scenes::ID::TEST;
	return Scenes::ID::TEST;
}

void SceneManager::OnAssetRenamed(const std::string& oldPath, const std::string& newPath)
{
	std::string normOld = NormalizeScenePath(oldPath);
	std::string normNew = NormalizeScenePath(newPath);

	if (m_activeScenePath == normOld)
	{
		m_activeScenePath = normNew;
	}
	if (m_nextScenePath == normOld)
	{
		m_nextScenePath = normNew;
	}

	for (auto& s : m_scenes)
	{
		if (s && s->GetJsonPath() == normOld)
		{
			s->SetJsonPath(normNew);
			s->SetName(fs::path(normNew).stem().string());
		}
	}

	RefreshSceneRegistry();
}

SceneManager& SceneManager::GetInstance()
{
	static SceneManager Instance;
	return Instance;
}

SceneManager::SceneManager(void)
{
	RefreshSceneRegistry();
	Init();
	EventManager::GetInstance().Init();
}

SceneManager::~SceneManager(void)
{
}

void SceneManager::Init()
{
	m_scenes.clear();

	m_fadeScene = std::make_shared<SceneFade>();
	m_fadeScene->Init();

	LoadSceneAdditive(INITIAL_SCENE, true);
}

void SceneManager::Uninit(void)
{
	UninitAndPop();
	m_scenes.clear();
	if (m_fadeScene)
	{
		m_fadeScene.reset();
	}
}

std::vector<std::string> SceneManager::GetAvailableSceneFiles() const
{
	std::vector<std::string> files;
	std::error_code ec;
	fs::path sceneDir("Assets/Scene");
	if (fs::exists(sceneDir, ec) && fs::is_directory(sceneDir, ec))
	{
		for (const auto& entry : fs::recursive_directory_iterator(sceneDir, ec))
		{
			if (!entry.is_regular_file(ec)) continue;
			auto ext = entry.path().extension();
			if (ext == ".json" || ext == ".scene")
			{
				std::string p = entry.path().generic_string();
				files.push_back(p);
			}
		}
	}
	std::sort(files.begin(), files.end());
	return files;
}

std::string SceneManager::GetActiveSceneName() const
{
	if (const CScene* active = GetActiveScene())
	{
		if (!active->GetName().empty()) return active->GetName();
	}
	if (!m_activeScenePath.empty())
	{
		return fs::path(m_activeScenePath).stem().string();
	}
	return "SceneTest";
}

std::shared_ptr<CScene> SceneManager::CreateSceneInstanceByPath(const std::string& jsonPath, Scenes::ID fallbackID)
{
	std::string normPath = NormalizeScenePath(jsonPath);
	Scenes::ID resolvedID = (fallbackID != Scenes::ID::NONE) ? fallbackID : ResolveSceneIDFromPath(normPath);
	return std::make_shared<CScene>(normPath, resolvedID);
}

std::shared_ptr<CScene> SceneManager::CreateSceneInstance(Scenes::ID _SceneID)
{
	if (_SceneID == Scenes::ID::NONE || _SceneID == Scenes::ID::Exit)
	{
		return nullptr;
	}
	std::string path = ResolveScenePathFromID(_SceneID);
	return CreateSceneInstanceByPath(path, _SceneID);
}

void SceneManager::LoadSceneAdditiveByPath(const std::string& jsonPath, bool setAsActive)
{
	std::string normPath = NormalizeScenePath(jsonPath);
	if (normPath.empty()) return;
	if (IsSceneLoadedByPath(normPath))
	{
		if (setAsActive)
		{
			m_activeScenePath = normPath;
			m_activeSceneID = ResolveSceneIDFromPath(normPath);
		}
		return;
	}

	auto newScene = CreateSceneInstanceByPath(normPath);
	if (newScene)
	{
		newScene->Init();
		m_scenes.push_back(newScene);

		if (setAsActive)
		{
			m_activeScenePath = normPath;
			m_activeSceneID = newScene->GetID();
		}
	}
}

void SceneManager::UnloadSceneByPath(const std::string& jsonPath)
{
	std::string normPath = NormalizeScenePath(jsonPath);
	auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
		[&normPath](const std::shared_ptr<CScene>& scene) {
			return scene && scene->GetJsonPath() == normPath;
		});

	if (it != m_scenes.end())
	{
		m_scenes.erase(it);
		if (m_activeScenePath == normPath)
		{
			if (!m_scenes.empty())
			{
				m_activeScenePath = m_scenes.back()->GetJsonPath();
				m_activeSceneID = m_scenes.back()->GetID();
			}
			else
			{
				m_activeScenePath.clear();
				m_activeSceneID = Scenes::ID::NONE;
			}
		}
	}
}

bool SceneManager::IsSceneLoadedByPath(const std::string& jsonPath) const
{
	std::string normPath = NormalizeScenePath(jsonPath);
	for (const auto& s : m_scenes)
	{
		if (s && s->GetJsonPath() == normPath) return true;
	}
	return false;
}

void SceneManager::ChangeSceneByPath(const std::string& jsonPath, bool withFade, float fadeDuration)
{
	std::string normPath = NormalizeScenePath(jsonPath);
	if (normPath.empty()) return;

	if (!withFade)
	{
		UninitAndPop();
		m_scenes.clear();
		LoadSceneAdditiveByPath(normPath, true);
		return;
	}

	if (m_transitionStep != TransitionStep::None) return;

	m_nextScenePath = normPath;
	m_nextSceneID = ResolveSceneIDFromPath(normPath);
	m_fadeDuration = fadeDuration;
	m_transitionStep = TransitionStep::FadeOut;

	if (m_fadeScene)
	{
		m_fadeScene->StartFadeOut(m_fadeDuration);
	}
}

bool SceneManager::CreateNewScene(const std::string& sceneName, SceneTemplateType templateType, std::string* outCreatedPath, const std::string& targetFolder)
{
	if (sceneName.empty()) return false;

	std::string normFolder = NormalizeFolderPath(targetFolder);
	fs::path inputPath(sceneName);
	if (inputPath.has_parent_path())
	{
		std::string sub = NormalizeFolderPath(inputPath.parent_path().generic_string());
		normFolder = normFolder.empty() ? ("Assets/Scene/" + sub) : (normFolder + "/" + sub);
		normFolder = NormalizeFolderPath(normFolder);
	}

	if (normFolder.empty() || (normFolder != "Assets/Scene" && normFolder.rfind("Assets/Scene/", 0) != 0))
	{
		normFolder = "Assets/Scene";
	}

	std::error_code ec;
	fs::create_directories(normFolder, ec);

	std::string cleanName = inputPath.stem().string();
	if (cleanName.empty()) return false;

	std::string targetPath = normFolder + "/" + cleanName + ".json";
	if (outCreatedPath) *outCreatedPath = targetPath;

	json rootJson;
	rootJson["sceneID"] = static_cast<int>(Scenes::ID::TEST);
	rootJson["eventSystem"]["firstSelected"] = "";
	rootJson["objects"] = json::array();

	auto makeTransform = [](float px, float py, float pz, float rx, float ry, float rz, float sx = 1.0f, float sy = 1.0f, float sz = 1.0f) {
		json t;
		t["position"] = { px, py, pz };
		t["rotation"] = { rx, ry, rz };
		t["scale"]    = { sx, sy, sz };
		return t;
	};

	// 1. Camera (always included)
	{
		json cam;
		cam["name"] = "Camera";
		cam["prefab"] = "Camera";
		cam["tag"] = 11;
		cam["type"] = "Camera";
		cam["transform"] = makeTransform(0.0f, 5.0f, -15.0f, 0.25f, 0.0f, 0.0f);
		rootJson["objects"].push_back(cam);
	}

	if (templateType != SceneTemplateType::EmptyUIScene)
	{
		// 2. Skydome
		{
			json sky;
			sky["name"] = "Skydome";
			sky["prefab"] = "Skydome";
			sky["tag"] = 4;
			sky["type"] = "Skydome";
			sky["transform"] = makeTransform(0.0f, 0.0f, 0.0f, 0.0f, 3.14f, 0.0f);
			rootJson["objects"].push_back(sky);
		}

		// 3. Field
		{
			json field;
			field["name"] = "Field";
			field["prefab"] = "Field";
			field["tag"] = 5;
			field["type"] = "Field";
			field["transform"] = makeTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			rootJson["objects"].push_back(field);
		}

		// 4. DirectionalLight
		{
			json light;
			light["name"] = "DirectionalLight";
			light["prefab"] = "DirectionalLight";
			light["tag"] = 12;
			light["type"] = "Light";
			light["transform"] = makeTransform(0.0f, 20.0f, -20.0f, 0.785f, 0.785f, 0.0f);
			light["light"] = {
				{ "ambientColor", { 0.25f, 0.25f, 0.30f } },
				{ "castShadow", true },
				{ "color", { 1.0f, 0.98f, 0.92f } },
				{ "intensity", 1.2f },
				{ "range", 100.0f },
				{ "shadowBias", 0.00025f },
				{ "shadowDarkness", 0.35f },
				{ "specularPower", 32.0f },
				{ "spotAngle", 45.0f },
				{ "type", 0 }
			};
			rootJson["objects"].push_back(light);
		}
	}

	if (templateType == SceneTemplateType::Standard3DTest || templateType == SceneTemplateType::ShadowShowcase || templateType == SceneTemplateType::ShadowLab)
	{
		// Player
		{
			json player;
			player["name"] = "Player";
			player["prefab"] = "Player";
			player["tag"] = 1;
			player["type"] = "Player";
			player["transform"] = (templateType == SceneTemplateType::ShadowLab)
				? makeTransform(0.0f, 0.0f, -8.0f, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f)
				: makeTransform(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
			rootJson["objects"].push_back(player);
		}

		if (templateType == SceneTemplateType::Standard3DTest)
		{
			json enemy;
			enemy["name"] = "Enemy";
			enemy["prefab"] = "Enemy";
			enemy["tag"] = 2;
			enemy["type"] = "Enemy";
			enemy["transform"] = makeTransform(0.0f, 0.0f, 10.0f, 0.0f, 3.14f, 0.0f);
			rootJson["objects"].push_back(enemy);
		}
		else if (templateType == SceneTemplateType::ShadowShowcase)
		{
			const float offsetsX[3] = { -4.5f, 0.0f, 4.5f };
			const float offsetsZ[3] = {  6.0f, 9.0f, 12.0f };
			for (int i = 0; i < 3; ++i)
			{
				json enemy;
				enemy["name"] = "ShadowCaster_Enemy_" + std::to_string(i + 1);
				enemy["prefab"] = "Enemy";
				enemy["tag"] = 2;
				enemy["type"] = "Enemy";
				enemy["transform"] = makeTransform(offsetsX[i], 0.0f, offsetsZ[i], 0.0f, 3.14f, 0.0f);
				rootJson["objects"].push_back(enemy);
			}
		}
		else if (templateType == SceneTemplateType::ShadowLab)
		{
			// Station 1: Acne & Peter Panning Lab (X: -9)
			{
				json ramp;
				ramp["name"] = "Acne_SlopedRamp";
				ramp["prefab"] = "ShadowCaster_Cube";
				ramp["tag"] = 3;
				ramp["type"] = "Enemy";
				ramp["transform"] = makeTransform(-9.0f, 0.9f, 0.0f, 0.35f, 0.0f, 0.0f, 3.5f, 0.25f, 6.0f);
				rootJson["objects"].push_back(ramp);
			}
			{
				json monk;
				monk["name"] = "Acne_MonkOrganic";
				monk["prefab"] = "Enemy";
				monk["tag"] = 3;
				monk["type"] = "Enemy";
				monk["transform"] = makeTransform(-9.0f, 0.0f, -4.5f, 0.0f, 0.0f, 0.0f, 0.7f, 0.7f, 0.7f);
				rootJson["objects"].push_back(monk);
			}
			{
				json cube;
				cube["name"] = "PeterPan_GroundCube";
				cube["prefab"] = "ShadowCaster_Cube";
				cube["tag"] = 3;
				cube["type"] = "Enemy";
				cube["transform"] = makeTransform(-9.0f, 0.5f, 5.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
				rootJson["objects"].push_back(cube);
			}

			// Station 2: PCSS Contact Hardening Lab (X: -2..2)
			{
				json pole;
				pole["name"] = "PCSS_LowPole";
				pole["prefab"] = "ShadowCaster_Cube";
				pole["tag"] = 3;
				pole["type"] = "Enemy";
				pole["transform"] = makeTransform(-2.0f, 0.75f, -2.0f, 0.0f, 0.0f, 0.0f, 0.5f, 1.5f, 0.5f);
				rootJson["objects"].push_back(pole);
			}
			{
				json pole;
				pole["name"] = "PCSS_MidPole";
				pole["prefab"] = "ShadowCaster_Cube";
				pole["tag"] = 3;
				pole["type"] = "Enemy";
				pole["transform"] = makeTransform(0.0f, 1.75f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 3.5f, 0.5f);
				rootJson["objects"].push_back(pole);
			}
			{
				json pole;
				pole["name"] = "PCSS_TallPole";
				pole["prefab"] = "ShadowCaster_Cube";
				pole["tag"] = 3;
				pole["type"] = "Enemy";
				pole["transform"] = makeTransform(2.0f, 3.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.5f, 6.0f, 0.5f);
				rootJson["objects"].push_back(pole);
			}
			{
				json bar;
				bar["name"] = "PCSS_TiltedBar";
				bar["prefab"] = "ShadowCaster_Cube";
				bar["tag"] = 3;
				bar["type"] = "Enemy";
				bar["transform"] = makeTransform(0.0f, 2.5f, 6.5f, 0.6f, 0.0f, 0.0f, 0.4f, 5.5f, 0.4f);
				rootJson["objects"].push_back(bar);
			}

			// Station 3: CSM Cascade Split Chain (X: 8)
			const struct { const char* name; float y; float z; float sy; } csmNodes[] = {
				{ "CSM_Near_1", 1.5f, -8.0f, 3.0f },
				{ "CSM_Near_2", 2.0f, -3.0f, 4.0f },
				{ "CSM_Mid_1",  2.5f,  4.0f, 5.0f },
				{ "CSM_Mid_2",  3.0f, 13.0f, 6.0f },
				{ "CSM_Far_1",  3.5f, 25.0f, 7.0f },
				{ "CSM_Far_2",  4.0f, 42.0f, 8.0f },
				{ "CSM_Far_3",  5.0f, 65.0f, 10.0f }
			};
			for (const auto& node : csmNodes)
			{
				json o;
				o["name"] = node.name;
				o["prefab"] = "ShadowCaster_Cube";
				o["tag"] = 3;
				o["type"] = "Enemy";
				o["transform"] = makeTransform(8.0f, node.y, node.z, 0.0f, 0.0f, 0.0f, 0.7f, node.sy, 0.7f);
				rootJson["objects"].push_back(o);
			}
		}
	}

	std::ofstream outFile(targetPath);
	if (!outFile.is_open()) return false;
	outFile << rootJson.dump(4);
	outFile.close();
	return true;
}

bool SceneManager::DuplicateScene(const std::string& srcJsonPath, const std::string& newSceneName, std::string* outCreatedPath, const std::string& targetFolder)
{
	std::string normSrc = NormalizeScenePath(srcJsonPath);
	if (normSrc.empty() || newSceneName.empty()) return false;

	std::error_code ec;
	if (!fs::exists(normSrc, ec)) return false;

	std::string normFolder = NormalizeFolderPath(targetFolder);
	if (normFolder.empty())
	{
		normFolder = NormalizeFolderPath(fs::path(normSrc).parent_path().generic_string());
	}

	fs::path inputPath(newSceneName);
	if (inputPath.has_parent_path())
	{
		std::string sub = NormalizeFolderPath(inputPath.parent_path().generic_string());
		normFolder = normFolder.empty() ? ("Assets/Scene/" + sub) : (normFolder + "/" + sub);
		normFolder = NormalizeFolderPath(normFolder);
	}

	if (normFolder.empty() || (normFolder != "Assets/Scene" && normFolder.rfind("Assets/Scene/", 0) != 0))
	{
		normFolder = "Assets/Scene";
	}

	fs::create_directories(normFolder, ec);
	std::string cleanName = inputPath.stem().string();
	if (cleanName.empty()) return false;

	std::string dstPath = normFolder + "/" + cleanName + ".json";
	if (outCreatedPath) *outCreatedPath = dstPath;

	fs::copy_file(normSrc, dstPath, fs::copy_options::overwrite_existing, ec);
	return !ec;
}

bool SceneManager::DeleteSceneFile(const std::string& jsonPath)
{
	std::string normPath = NormalizeScenePath(jsonPath);
	if (normPath.empty()) return false;

	// Prevent deleting the currently active scene or core default scenes if only 1 scene remains
	auto allScenes = GetAvailableSceneFiles();
	if (allScenes.size() <= 1) return false;

	std::error_code ec;
	if (!fs::exists(normPath, ec)) return false;

	bool wasActive = (m_activeScenePath == normPath);
	UnloadSceneByPath(normPath);

	bool removed = fs::remove(normPath, ec);
	if (removed && wasActive)
	{
		auto remaining = GetAvailableSceneFiles();
		if (!remaining.empty())
		{
			ChangeSceneByPath(remaining[0], false);
		}
	}
	return removed && !ec;
}

void SceneManager::LoadSceneAdditive(Scenes::ID _SceneID, bool setAsActive)
{
	std::string path = ResolveScenePathFromID(_SceneID);
	if (IsSceneLoadedByPath(path)) return;

	auto newScene = CreateSceneInstance(_SceneID);
	if (newScene)
	{
		newScene->Init();
		m_scenes.push_back(newScene);

		if (setAsActive)
		{
			m_activeSceneID = _SceneID;
			m_activeScenePath = newScene->GetJsonPath();
		}
	}
}

void SceneManager::UnloadScene(Scenes::ID _SceneID)
{
	auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
		[_SceneID](const std::shared_ptr<CScene>& scene) {
			return scene && scene->GetID() == _SceneID;
		});

	if (it != m_scenes.end())
	{
		std::string unloadedPath = (*it)->GetJsonPath();
		m_scenes.erase(it);
		if (m_activeSceneID == _SceneID || m_activeScenePath == unloadedPath)
		{
			if (!m_scenes.empty())
			{
				m_activeSceneID = m_scenes.back()->GetID();
				m_activeScenePath = m_scenes.back()->GetJsonPath();
			}
			else
			{
				m_activeSceneID = Scenes::ID::NONE;
				m_activeScenePath.clear();
			}
		}
	}
}

void SceneManager::SetActiveScene(Scenes::ID _SceneID)
{
	for (const auto& s : m_scenes)
	{
		if (s && s->GetID() == _SceneID)
		{
			m_activeSceneID = _SceneID;
			m_activeScenePath = s->GetJsonPath();
			break;
		}
	}
}

bool SceneManager::IsSceneLoaded(Scenes::ID _SceneID) const
{
	for (const auto& s : m_scenes)
	{
		if (s && s->GetID() == _SceneID) return true;
	}
	return false;
}

void SceneManager::ChangeSceneWithFade(Scenes::ID nextSceneID, float fadeDuration)
{
	if (m_transitionStep != TransitionStep::None) return;

	m_nextSceneID = nextSceneID;
	m_nextScenePath = (nextSceneID == Scenes::ID::Exit) ? "" : ResolveScenePathFromID(nextSceneID);
	m_fadeDuration = fadeDuration;
	m_transitionStep = TransitionStep::FadeOut;

	if (m_fadeScene)
	{
		m_fadeScene->StartFadeOut(m_fadeDuration);
	}
}

void SceneManager::ChangeSceneInstant(Scenes::ID _SceneID)
{
	UninitAndPop();
	m_scenes.clear();
	LoadSceneAdditive(_SceneID, true);
}

void SceneManager::ChangeScene(Scenes::ID _SceneID)
{
	ChangeSceneWithFade(_SceneID, 0.4f);
}

void SceneManager::PushScene(Scenes::ID _SceneID)
{
	LoadSceneAdditive(_SceneID, true);
}

void SceneManager::PopScene(void)
{
	if (!m_scenes.empty())
	{
		UnloadSceneByPath(m_scenes.back()->GetJsonPath());
	}
}

void SceneManager::UninitAndPop(void)
{
	DX12Manager::GetInstance().WaitForPendingOperations();
	ObjectManager::GetInstance().Uninit();
	TextureManager::GetInstance().Clear();
	ModelManager::GetInstance().Clear();
	ButtonEventManager::GetInstance().ClearSelectedGameObject();
	CInspectorUI::GetInstance().ClearSelection();
}

void SceneManager::ProcessTransition()
{
	if (m_transitionStep == TransitionStep::None) return;

	switch (m_transitionStep)
	{
	case TransitionStep::FadeOut:
		if (!m_fadeScene || !m_fadeScene->IsFading())
		{
			m_transitionStep = TransitionStep::SwapScene;
		}
		break;

	case TransitionStep::SwapScene:
		if (m_nextSceneID == Scenes::ID::Exit)
		{
			IsGameEnd = true;
			m_transitionStep = TransitionStep::None;
			return;
		}

		UninitAndPop();
		m_scenes.clear();

		if (!m_nextScenePath.empty())
		{
			LoadSceneAdditiveByPath(m_nextScenePath, true);
		}
		else
		{
			LoadSceneAdditive(m_nextSceneID, true);
		}

		m_transitionStep = TransitionStep::FadeIn;
		if (m_fadeScene)
		{
			m_fadeScene->StartFadeIn(m_fadeDuration);
		}
		break;

	case TransitionStep::FadeIn:
		if (!m_fadeScene || !m_fadeScene->IsFading())
		{
			m_transitionStep = TransitionStep::None;
		}
		break;

	default:
		break;
	}
}

void SceneManager::Update()
{
	ProcessSceneEvents();

	if (IsGameEnd) return;

	ProcessTransition();

	if (m_fadeScene)
	{
		m_fadeScene->Update();
	}

	for (auto& s : m_scenes)
	{
		s->Update();
	}
}

void SceneManager::Draw(void)
{
	ObjectManager::GetInstance().FlushDestroyedObjects();

	for (auto& s : m_scenes)
	{
		s->Draw();
	}

	if (m_fadeScene)
	{
		m_fadeScene->Draw();
	}
}

void SceneManager::ProcessSceneEvents()
{
	static const Events::ID sceneEventIDs[] = {
		Events::ID::ChangeScene,
		Events::ID::PushScene,
		Events::ID::PopScene
	};

	Event* pEvent = nullptr;
	for (auto id : sceneEventIDs)
	{
		pEvent = EventManager::GetInstance().FindEventByID(id);
		if (pEvent) break;
	}

	if (!pEvent) return;

	switch (pEvent->GetEventID())
	{
	case Events::ID::ChangeScene:
	{
		auto* nextData = static_cast<EventData_NextScene*>(pEvent->GetEventData());
		if (nextData)
		{
			if (nextData->HasScenePath())
			{
				ChangeSceneByPath(nextData->GetNextScenePath(), true, 0.4f);
			}
			else
			{
				ChangeSceneWithFade(nextData->GetNextScene(), 0.4f);
			}
		}
		break;
	}
	default:
		break;
	}

	EventManager::GetInstance().ClearEvents();
}
