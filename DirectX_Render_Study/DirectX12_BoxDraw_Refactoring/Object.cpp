//===== インクルード =====
#include "Object.h"

//オブジェクトの情報
#include "ObjectInfo.h"


//===== 関数定義 =====

//コンストラクタ
//基本のコンポーネントの作成
CObject::CObject()
	:m_IsValid(true)
{
	//オブジェクト情報のコンポーネントの作成
	AddComponent<CObjectInfo>();
}

//オブジェクトの名前のセット
void CObject::SetName(String _ObjectName)
{
	CObjectInfo* objectInfo = GetComponent<CObjectInfo>();
	objectInfo->SetObjectName(_ObjectName);
}
