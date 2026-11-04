#pragma once
#include <string>
#include <memory>
#include "SceneEnums.h"

using String = std::string;

class RenderPipeline;
class RenderTexture;

//===== クラス定義 =====
// 全シーン共通のデータ駆動型シーンクラス (JSONベース)
class CScene
{
public:
	CScene() = default;
	CScene(Scenes::ID _id);
	CScene(const std::string& jsonPath, Scenes::ID _id = Scenes::ID::NONE);
	virtual ~CScene();

	virtual void Init();
	virtual void Update();
	virtual void Draw();

	// レンダリングパイプラインへのアクセサ
	virtual RenderPipeline* GetRenderPipeline() { return m_renderPipeline.get(); }

	// シーン用レンダーターゲットへのアクセサ
	virtual RenderTexture* GetSceneTexture() { return m_pSceneTexture.get(); }

	//----- Getter / Setter -----
	Scenes::ID GetID() const { return id; }
	const std::string& GetName() const { return m_Name; }
	void SetName(const std::string& name) { m_Name = name; }
	const std::string& GetJsonPath() const { return m_jsonPath; }
	void SetJsonPath(const std::string& path);

protected:
	void ApplySceneUIEntranceEffects();
	void BuildFallbackSceneIfMissing();

	String m_Name;
	String m_jsonPath;
	Scenes::ID id = Scenes::ID::NONE;

	std::unique_ptr<RenderPipeline> m_renderPipeline;
	std::unique_ptr<RenderTexture> m_pSceneTexture;
};
