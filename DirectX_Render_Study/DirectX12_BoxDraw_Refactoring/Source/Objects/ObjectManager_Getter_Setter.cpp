//ObjectManager_Getter_Setter.cpp

//Getter,Setter関連の処理

//====== インクルード =====

//ヘッダ
#include "ObjectManager.h"

//コンポーネント
#include "ObjectInfo.h"

//オブジェクト
#include "Player.h"
#include "Camera.h"



//====== メソッド定義 =====

//----- Player -----
Player* ObjectManager::GetPlayer()
{
	//何も無いならnullptr
	if (vecObject[Object::objectTag::PLAYER].size() < 1)return nullptr;


	return (Player*)(vecObject[Object::objectTag::PLAYER][0].get());
}

//----- Camera -----
//Todo : 複数あるカメラを取得できるようにする
Camera* ObjectManager::GetCamera()
{
	//何も無いならnullptr
	if (vecObject[Object::objectTag::CAMERA].size() < 1)return nullptr;


	return (Camera*)(vecObject[Object::objectTag::CAMERA][0].get());
}

//----- Manager -----
//名前で探して,Getする
//将来的には、ObjectInfoで探して、Getすればいいか？
CObject* ObjectManager::GetManager(String name)
{
	for (auto& object : vecObject[Object::objectTag::MANAGER])
	{
		String str = object->GetComponent<CObjectInfo>()->GetObjectName();

		//名前をgetする
		if (name == str)
		{
			//一致していたら返す
			return object.get();
		}
	}


	return nullptr;
}