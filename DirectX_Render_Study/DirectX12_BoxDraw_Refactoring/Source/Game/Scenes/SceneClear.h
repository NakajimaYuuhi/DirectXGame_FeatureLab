#pragma once
#include "Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

#include "RenderPipeline.h"
#include "RenderTexture.h"

class SceneClear :
    public CScene
{
public:
	SceneClear();

	~SceneClear();

	void Init();

	void Update();

	void Draw();

	RenderPipeline* GetRenderPipeline() override { return m_renderPipeline.get(); }

private:
	std::unique_ptr<RenderPipeline> m_renderPipeline;
	std::unique_ptr<RenderTexture> m_renderTexture;
	std::unique_ptr<RenderTexture> m_pOffscreenTexture;

};

