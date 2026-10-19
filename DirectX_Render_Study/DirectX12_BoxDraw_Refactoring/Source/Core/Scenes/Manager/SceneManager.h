#pragma once
#include "EngineAPI.h"
#include "../Base/Scene.h"
#include "SceneEnums.h"
#include "SmartPtrAlias.h"
#include <vector>
#include <memory>

class SceneFade;

enum class TransitionStep
{
	None,
	FadeOut,
	SwapScene,
	FadeIn
};

class SceneManager
{
public:
	ENGINE_API void Init(void);
	ENGINE_API void Uninit(void);
	ENGINE_API void Update(void);
	ENGINE_API void Draw(void);

	// 並列シーン操作 (Additive Scene API)
	ENGINE_API void LoadSceneAdditive(Scenes::ID _SceneID, bool setAsActive = true);
	ENGINE_API void UnloadScene(Scenes::ID _SceneID);
	ENGINE_API void SetActiveScene(Scenes::ID _SceneID);
	ENGINE_API bool IsSceneLoaded(Scenes::ID _SceneID) const;

	// フェード付きシーン切り替え
	ENGINE_API void ChangeSceneWithFade(Scenes::ID nextSceneID, float fadeDuration = 0.5f);
	ENGINE_API void ChangeSceneInstant(Scenes::ID _SceneID);

	// 互換用シーン切り替え
	ENGINE_API void ChangeScene(Scenes::ID _SceneID);

	// 旧API互換
	ENGINE_API void PushScene(Scenes::ID _SceneID);
	ENGINE_API void PopScene(void);

private:
	std::vector<std::shared_ptr<CScene>> m_scenes;
	Scenes::ID m_activeSceneID = Scenes::ID::NONE;

	std::shared_ptr<SceneFade> m_fadeScene;

	TransitionStep m_transitionStep = TransitionStep::None;
	Scenes::ID m_nextSceneID = Scenes::ID::NONE;
	float m_fadeDuration = 0.5f;

	bool IsGameEnd = false;

private:
	std::shared_ptr<CScene> CreateSceneInstance(Scenes::ID _SceneID);
	void UninitAndPop();
	void ProcessSceneEvents();
	void ProcessTransition();

// Getter, Setter
public:
	bool GetIsGameEnd(void) const { return IsGameEnd; }
	Scenes::ID GetActiveSceneID() const { return m_activeSceneID; }
	bool IsTransitioning() const { return m_transitionStep != TransitionStep::None; }
	CScene* GetActiveScene() const
	{
		for (const auto& s : m_scenes) {
			if (s && s->GetID() == m_activeSceneID) return s.get();
		}
		return nullptr;
	}

// シングルトン
public:
	ENGINE_API static SceneManager& GetInstance();

private:
	SceneManager();
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
};
