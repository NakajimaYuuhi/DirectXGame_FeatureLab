//===== インクルード =====
#pragma once
#include "Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

#include "RenderPipeline.h"
#include "RenderTexture.h"

//===== 前方宣言 =====
class CObject;

//===== クラス定義 =====
class SceneTitle : public CScene
{
public:
	SceneTitle();

	~SceneTitle();

	void Init();

	void Update();

	void Draw();


private:
	std::unique_ptr<RenderPipeline> m_renderPipeline;
	std::unique_ptr<RenderTexture> m_renderTexture;
	std::unique_ptr<RenderTexture> m_pOffscreenTexture;
};

