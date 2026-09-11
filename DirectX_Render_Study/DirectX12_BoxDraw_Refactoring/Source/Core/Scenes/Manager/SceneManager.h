#pragma once
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
	void Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	// 並列シーン操作 (Additive Scene API)
	void LoadSceneAdditive(Scenes::ID _SceneID, bool setAsActive = true);
	void UnloadScene(Scenes::ID _SceneID);
	void SetActiveScene(Scenes::ID _SceneID);
	bool IsSceneLoaded(Scenes::ID _SceneID) const;

	// フェード付きシーン切り替え
	void ChangeSceneWithFade(Scenes::ID nextSceneID, float fadeDuration = 0.5f);

	// 互換用シーン切り替え
	void ChangeScene(Scenes::ID _SceneID);

	// 旧API互換
	void PushScene(Scenes::ID _SceneID);
	void PopScene(void);

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

// シングルトン
public:
	static SceneManager& GetInstance()
	{
		static SceneManager Instance;
		return Instance;
	}

private:
	SceneManager();
	~SceneManager();
	SceneManager(const SceneManager&) = delete;
	SceneManager& operator=(const SceneManager&) = delete;
};