#pragma once
#include <string>
#include "SceneEnums.h"

using String = std::string;

//===== ?O???? =====
//class CObject;

//===== ?N???X??` =====
class CScene
{
public:

	//
	CScene()=  default;
	CScene(Scenes::ID _id) { id = _id; }
	virtual ~CScene() = default;

	virtual void Init()		= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

	//----- Getter -----
	Scenes::ID GetID()const { return id; }

protected:
	String m_Name;
	//Tag
	Scenes::ID id;

private:
};

