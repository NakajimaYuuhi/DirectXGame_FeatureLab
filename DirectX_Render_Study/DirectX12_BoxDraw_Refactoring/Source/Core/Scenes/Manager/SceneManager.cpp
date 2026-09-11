#include "SceneManager.h"
#include "ObjectManager.h"
#include "../Base/Scene.h"
#include "../Instances/SceneTest.h"
#include "../Instances/SceneTitle.h"
#include "../Instances/SceneClear.h"
#include "../Instances/SceneFailed.h"
#include "../Instances/SceneFade.h"
#include "ButtonEventManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "Event.h"
#include "EventData_NextScene.h"
#include "TextureManager.h"
#include "ModelManager.h"
#include "DX12Manager.h"
#include <algorithm>

SceneManager::SceneManager(void)
{
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

std::shared_ptr<CScene> SceneManager::CreateSceneInstance(Scenes::ID _SceneID)
{
	switch (_SceneID)
	{
	case Scenes::ID::TEST:
		return std::make_shared<CSceneTest>();
	case Scenes::ID::TITLE:
		return std::make_shared<SceneTitle>();
	case Scenes::ID::Clear:
		return std::make_shared<SceneClear>();
	case Scenes::ID::Failed:
		return std::make_shared<SceneFailed>();
	default:
		return nullptr;
	}
}

void SceneManager::LoadSceneAdditive(Scenes::ID _SceneID, bool setAsActive)
{
	if (IsSceneLoaded(_SceneID)) return;

	auto newScene = CreateSceneInstance(_SceneID);
	if (newScene)
	{
		newScene->Init();
		m_scenes.push_back(newScene);

		if (setAsActive)
		{
			m_activeSceneID = _SceneID;
		}
	}
}

void SceneManager::UnloadScene(Scenes::ID _SceneID)
{
	auto it = std::find_if(m_scenes.begin(), m_scenes.end(),
		[_SceneID](const std::shared_ptr<CScene>& scene) {
			return scene->GetID() == _SceneID;
		});

	if (it != m_scenes.end())
	{
		m_scenes.erase(it);
		if (m_activeSceneID == _SceneID)
		{
			m_activeSceneID = m_scenes.empty() ? Scenes::ID::NONE : m_scenes.back()->GetID();
		}
	}
}

void SceneManager::SetActiveScene(Scenes::ID _SceneID)
{
	if (IsSceneLoaded(_SceneID))
	{
		m_activeSceneID = _SceneID;
	}
}

bool SceneManager::IsSceneLoaded(Scenes::ID _SceneID) const
{
	for (const auto& s : m_scenes)
	{
		if (s->GetID() == _SceneID) return true;
	}
	return false;
}

void SceneManager::ChangeSceneWithFade(Scenes::ID nextSceneID, float fadeDuration)
{
	if (m_transitionStep != TransitionStep::None) return;

	m_nextSceneID = nextSceneID;
	m_fadeDuration = fadeDuration;
	m_transitionStep = TransitionStep::FadeOut;

	if (m_fadeScene)
	{
		m_fadeScene->StartFadeOut(m_fadeDuration);
	}
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
		UnloadScene(m_scenes.back()->GetID());
	}
}

void SceneManager::UninitAndPop(void)
{
	DX12Manager::GetInstance().WaitForPendingOperations();
	ObjectManager::GetInstance().Uninit();
	TextureManager::GetInstance().Clear();
	ModelManager::GetInstance().Clear();
	ButtonEventManager::GetInstance().ClearSelectedGameObject();
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

		LoadSceneAdditive(m_nextSceneID, true);

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
		ChangeSceneWithFade(
			(static_cast<EventData_NextScene*>(pEvent->GetEventData())->GetNextScene()),
			0.4f
		);
		break;
	}

	EventManager::GetInstance().ClearEvents();
}