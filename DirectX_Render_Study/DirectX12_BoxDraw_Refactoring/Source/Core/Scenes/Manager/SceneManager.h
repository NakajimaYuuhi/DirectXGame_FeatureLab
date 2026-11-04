#pragma once
#include "EngineAPI.h"
#include "../Base/Scene.h"
#include "SceneEnums.h"
#include "SmartPtrAlias.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

class SceneFade;

enum class TransitionStep
{
	None,
	FadeOut,
	SwapScene,
	FadeIn
};

// シーン新規作成テンプレート種別
enum class SceneTemplateType
{
	Standard3DTest = 0, // Camera + DirectionalLight + Field + Skydome + Player + Enemy
	ShadowShowcase = 1, // Camera + DirectionalLight + Field + Skydome + Player + 3 Staggered Enemies
	Minimal3D      = 2, // Camera + DirectionalLight + Field + Skydome
	EmptyUIScene   = 3  // Camera only
};

class SceneManager
{
public:
	ENGINE_API void Init(void);
	ENGINE_API void Uninit(void);
	ENGINE_API void Update(void);
	ENGINE_API void Draw(void);

	// =========================================================
	// データ駆動型シーン操作 API (JSONパス / シーン名ベース)
	// =========================================================
	ENGINE_API std::vector<std::string> GetAvailableSceneFiles() const;
	ENGINE_API void ChangeSceneByPath(const std::string& jsonPath, bool withFade = false, float fadeDuration = 0.4f);
	ENGINE_API void LoadSceneAdditiveByPath(const std::string& jsonPath, bool setAsActive = true);
	ENGINE_API void UnloadSceneByPath(const std::string& jsonPath);
	ENGINE_API bool IsSceneLoadedByPath(const std::string& jsonPath) const;

	// エディタからのシーン作成・複製・削除
	ENGINE_API bool CreateNewScene(const std::string& sceneName, SceneTemplateType templateType, std::string* outCreatedPath = nullptr);
	ENGINE_API bool DuplicateScene(const std::string& srcJsonPath, const std::string& newSceneName, std::string* outCreatedPath = nullptr);
	ENGINE_API bool DeleteSceneFile(const std::string& jsonPath);

	// 既存の Scenes::ID と JSON パスの相互変換ヘルパー
	ENGINE_API static std::string ResolveScenePathFromID(Scenes::ID sceneID);
	ENGINE_API static Scenes::ID ResolveSceneIDFromPath(const std::string& jsonPath);

	// Dynamic Scene Registry & Rename notification
	ENGINE_API static void RefreshSceneRegistry();
	ENGINE_API void OnAssetRenamed(const std::string& oldPath, const std::string& newPath);

	// =========================================================
	// 並列シーン操作 & 旧 Scenes::ID 互換 API
	// =========================================================
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
	std::string m_activeScenePath = "Assets/Scene/SceneTest.json";

	std::shared_ptr<SceneFade> m_fadeScene;

	TransitionStep m_transitionStep = TransitionStep::None;
	Scenes::ID m_nextSceneID = Scenes::ID::NONE;
	std::string m_nextScenePath;
	float m_fadeDuration = 0.5f;

	bool IsGameEnd = false;

	mutable std::unordered_map<int, std::string> m_sceneIdToPath;
	mutable std::unordered_map<std::string, Scenes::ID> m_pathToSceneId;

private:
	std::shared_ptr<CScene> CreateSceneInstance(Scenes::ID _SceneID);
	std::shared_ptr<CScene> CreateSceneInstanceByPath(const std::string& jsonPath, Scenes::ID fallbackID = Scenes::ID::NONE);
	void UninitAndPop();
	void ProcessSceneEvents();
	void ProcessTransition();

// Getter, Setter
public:
	bool GetIsGameEnd(void) const { return IsGameEnd; }
	Scenes::ID GetActiveSceneID() const { return m_activeSceneID; }
	const std::string& GetActiveScenePath() const { return m_activeScenePath; }
	std::string GetActiveSceneName() const;
	bool IsTransitioning() const { return m_transitionStep != TransitionStep::None; }

	CScene* GetActiveScene() const
	{
		for (const auto& s : m_scenes)
		{
			if (s && !m_activeScenePath.empty() && s->GetJsonPath() == m_activeScenePath)
				return s.get();
		}
		for (const auto& s : m_scenes)
		{
			if (s && s->GetID() == m_activeSceneID)
				return s.get();
		}
		if (!m_scenes.empty())
		{
			return m_scenes.back().get();
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
