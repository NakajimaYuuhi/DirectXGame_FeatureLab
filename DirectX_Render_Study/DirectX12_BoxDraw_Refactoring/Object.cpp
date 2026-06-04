//===== インクルード =====
#include "Object.h"

//オブジェクトの情報
#include "ObjectInfo.h"

#include "Component.h"
//===== 関数定義 =====

//コンストラクタ
//基本のコンポーネントの作成

//各子オブジェクトは外部データから、コンポーネント作成できるといいかも
CObject::CObject()
	:isValid(true)
{
	//オブジェクト情報のコンポーネントの作成
	AddComponent<CObjectInfo>();
}

//デストラクタ エラー回避のためにここで定義
CObject::~CObject() = default;

void CObject::Init() 
{
	//各コンポーネントのInitを呼ぶ
	for (auto& c : components)
	{
		//コンポーネントのInitを呼ぶ
		c->Init();
	}
}

//オブジェクトの名前のセット
void CObject::SetName(String _ObjectName)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectName(_ObjectName);
}
