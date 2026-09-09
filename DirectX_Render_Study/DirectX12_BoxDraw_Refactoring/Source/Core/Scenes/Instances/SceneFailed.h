#pragma once
#include "../Base/Scene.h"

#include "SmartPtrAlias.h"
#include "ContainerAlias.h"

#include "RenderPipeline.h"
#include "RenderTexture.h"

class SceneFailed :
    public CScene
{
public:
	SceneFailed();

	~SceneFailed();

	void Init();

	void Update();

	void Draw();

private:
	std::unique_ptr<RenderPipeline> m_renderPipeline;
	std::unique_ptr<RenderTexture> m_renderTexture;
	std::unique_ptr<RenderTexture> m_pOffscreenTexture;

};

