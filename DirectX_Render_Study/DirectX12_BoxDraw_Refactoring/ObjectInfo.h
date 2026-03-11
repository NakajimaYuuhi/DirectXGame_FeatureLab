#pragma once
#include "Component.h"
#include <cassert>

#include <string>
using String = std::string;

//クラス定義
class CObjectInfo :public CComponent
{
public:
	CObjectInfo() 
		:m_ObjectName("")			//指定が無いなら空
		,CComponent("ObjectInfo")	//基本はコンポーネントの種類 
	{}

	//Object名を指定する
	CObjectInfo(String _ObjectName)
		: m_ObjectName(_ObjectName)	//指定した文字列
		, CComponent("ObjectInfo")	//基本はコンポーネントの種類 
	{}


private:
	String	m_ObjectName;	//オブジェクトの名前


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
	void SetObjectName(String _ObjectName) { m_ObjectName = _ObjectName; }

	String GetObjectName() { return m_ObjectName; }

};

