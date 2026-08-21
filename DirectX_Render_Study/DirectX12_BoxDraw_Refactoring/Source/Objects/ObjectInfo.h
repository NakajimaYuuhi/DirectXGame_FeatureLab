#pragma once
#include "ObjectTag.h"

#include "Component.h"
#include <cassert>

#include "StringAlias.h"

//クラス定義
class CObjectInfo :public CComponent
{
public:
	CObjectInfo() 
		:objectName("")			//指定が無いなら空
		,CComponent("ObjectInfo")	//基本はコンポーネントの種類 
	{}

	//Object名を指定する
	CObjectInfo(String _ObjectName)
		: objectName(_ObjectName)	//指定した文字列
		, CComponent("ObjectInfo")	//基本はコンポーネントの種類 
	{}


private:
	String	objectName;	//オブジェクトの名前
	ObjectTag objectTag = ObjectTag::NONE;	//オブジェクトのタグ
	


	//----- 特別に書いておくGetter,Setter -----
public:

	//このコンポーネントを無効にされたら困るので、書いておく
	void SetIsValid(bool _ComponentIsValid) 
	{ 
		if (!_ComponentIsValid)
		{
			/*無効にできないコンポーネントなのでエラーを出す*/
			assert(false && "Attempted to disable a locked component / ObjectInfoコンポーネントは無効にできません");
			return; 
		}
		m_ComponentIsValid = _ComponentIsValid;
	}
	//他のGetter,Setterは下にある

	//----- 当たり前に書くGetter,Setter -----
public:
	void SetObjectName(String _ObjectName) { objectName = _ObjectName; }

	String GetObjectName() { return objectName; }

	void SetObjectTag(ObjectTag _ObjectTag) { objectTag = _ObjectTag; }

	ObjectTag GetObjectTag() { return objectTag; }

};

