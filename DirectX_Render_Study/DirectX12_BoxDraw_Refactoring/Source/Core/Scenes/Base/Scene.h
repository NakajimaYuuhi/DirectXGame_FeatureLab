#pragma once
#include <string>
#include "SceneEnums.h"

using String = std::string;

class RenderPipeline;

//===== クラス定義 =====
class CScene
{
public:
	CScene() = default;
	CScene(Scenes::ID _id) { id = _id; }
	virtual ~CScene() = default;

	virtual void Init()		= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

	// レンダリングパイプラインへのアクセサ
	virtual RenderPipeline* GetRenderPipeline() { return nullptr; }

	//----- Getter -----
	Scenes::ID GetID() const { return id; }

protected:
	String m_Name;
	Scenes::ID id = Scenes::ID::NONE;

private:
};
