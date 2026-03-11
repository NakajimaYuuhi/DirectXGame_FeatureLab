#pragma once
#include <string>

using String = std::string;

//===== 前方宣言 =====
//class CObject;

//===== クラス定義 =====
class CScene
{
public:

	//
	CScene()=  default;
	virtual ~CScene() = default;

	virtual void Init()		= 0;
	virtual void Update()	= 0;
	virtual void Draw()		= 0;

protected:
	String m_Name;
	//Tag


private:
};

