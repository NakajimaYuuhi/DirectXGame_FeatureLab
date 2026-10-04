//ObjectManager_Getter_Setter.cpp

//Getter,Setter関連の処理

//====== インクルード =====

//ヘッダ
#include "ObjectManager.h"

//コンポーネント
#include "ObjectInfo.h"
#include "CameraComponent.h"

//オブジェクト
#include "FieldComponent.h"



//====== メソッド定義 =====

void ObjectManager::AddObject(ObjectTag _Tag, CObject* _Object)
{
	if (!_Object) return;
	m_pendingAddObjects.push_back({ _Tag, UniquePtr<CObject>(_Object) });
}

void ObjectManager::FlushPendingAddObjects()
{
	if (m_pendingAddObjects.empty()) return;

	for (auto& pair : m_pendingAddObjects)
	{
		int tagIdx = static_cast<int>(pair.first);
		if (tagIdx >= 0 && tagIdx < static_cast<int>(vecObject.size()) && pair.second)
		{
			vecObject[tagIdx].push_back(std::move(pair.second));
		}
	}
	m_pendingAddObjects.clear();
}

//----- Player -----
CObject* ObjectManager::GetPlayer()
{
	if (vecObject[Object::objectTag::PLAYER].size() >= 1)
	{
		return vecObject[Object::objectTag::PLAYER][0].get();
	}

	for (const auto& pair : m_pendingAddObjects)
	{
		if (pair.second && !pair.second->GetIsDestroyed() && pair.first == ObjectTag::PLAYER)
		{
			return pair.second.get();
		}
	}

	return nullptr;
}

//----- Camera -----
CObject* ObjectManager::GetCameraObject()
{
	const auto& cameras = vecObject[static_cast<int>(ObjectTag::CAMERA)];
	for (const auto& obj : cameras)
	{
		if (obj && !obj->GetIsDestroyed())
		{
			return obj.get();
		}
	}

	for (const auto& vec : vecObject)
	{
		for (const auto& obj : vec)
		{
			if (obj && !obj->GetIsDestroyed())
			{
				if (obj->GetComponent<CameraComponent>() )
				{
					return obj.get();
				}
			}
		}
	}

	for (const auto& pair : m_pendingAddObjects)
	{
		if (pair.second && !pair.second->GetIsDestroyed())
		{
			if (pair.first == ObjectTag::CAMERA || pair.second->GetComponent<CameraComponent>() )
			{
				return pair.second.get();
			}
		}
	}

	return nullptr;
}

CameraComponent* ObjectManager::GetCamera()
{
	CObject* camObj = GetCameraObject();
	if (camObj)
	{
		return camObj->GetComponent<CameraComponent>();
	}
	return nullptr;
}

CameraComponent* ObjectManager::GetCameraComponent()
{
	CObject* camObj = GetCameraObject();
	if (camObj)
	{
		return camObj->GetComponent<CameraComponent>();
	}
	return nullptr;
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

//----- Field -----
FieldComponent* ObjectManager::GetField()
{
	const auto& fields = vecObject[Object::objectTag::FIELD];
	for (const auto& obj : fields)
	{
		if (obj && !obj->GetIsDestroyed())
		{
			if (auto fc = obj->GetComponent<FieldComponent>()) return fc;
		}
	}
	return nullptr;
}