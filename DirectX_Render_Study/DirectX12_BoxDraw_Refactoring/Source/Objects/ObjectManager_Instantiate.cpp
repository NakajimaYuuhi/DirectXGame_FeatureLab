//ObjectManager_Getter_Setter.cpp

//Instantiate?????

//====== ?C???N???[?h =====

//?w?b?_
#include "ObjectManager.h"
#include "PrefabManager.h"
#include "ObjectInfo.h"

//?I?u?W?F?N?g

// --3D

//bullet

//billboard

//effect

//field
#include "FieldComponent.h"

//camera
#include "Camera.h"


// --2D
#include "TextRenderer.h"

// --Manager
#include "EnemyCounterComponent.h"


//===== ???\?b?h??` =====
CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
{
	return Instantiate(_SceneID, _Tag, _TypeName, _TypeName);
}

CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName, std::string _ObjectName)
{
	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
	CObject* returnObject = nullptr;

	// 1. Try dynamic PrefabManager lookup first (Data-driven prefabs)
	if (PrefabManager::GetInstance().HasPrefab(_TypeName))
	{
		CObject* rawObj = PrefabManager::GetInstance().Instantiate(_TypeName, _ObjectName);
		if (rawObj)
		{
			tmpObject = std::unique_ptr<CObject>(rawObj);
			returnObject = tmpObject.get();

			ObjectTag finalTag = _Tag;
			if (auto info = rawObj->GetComponent<CObjectInfo>())
			{
				if (info->GetObjectTag() != ObjectTag::NONE)
				{
					finalTag = info->GetObjectTag();
				}
				else
				{
					info->SetObjectTag(_Tag);
				}
			}

			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(finalTag), std::move(tmpObject) });
			return returnObject;
		}
	}

	if (_Tag == ObjectTag::NONE && (_TypeName == "Camera" || _ObjectName == "Camera" || _TypeName == "CameraComponent"))
	{
		_Tag = ObjectTag::CAMERA;
	}

	// 2. Fallback to C++ class-based instantiation
	switch (_Tag)
	{
	case ObjectTag::NONE:
		if (_TypeName == "Camera" || _ObjectName == "Camera" || _TypeName == "CameraComponent")
		{
			tmpObject = std::make_unique<Camera>(_ObjectName);
			returnObject = tmpObject.get();
			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::CAMERA), std::move(tmpObject) });
			break;
		}
		tmpObject = std::make_unique<CObject>(_ObjectName);
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::FIELD), std::move(tmpObject) });
		break;

	case ObjectTag::BACKGROUND:
		{
			CObject* rawObj = PrefabManager::GetInstance().Instantiate("SkydomeJSON", _ObjectName);
			if (!rawObj) rawObj = PrefabManager::GetInstance().InstantiateFromJSON("Assets/Prefabs/Skydome.json", _ObjectName);
			tmpObject = std::unique_ptr<CObject>(rawObj);
			returnObject = tmpObject.get();
			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::BACKGROUND), std::move(tmpObject) });
		}
		break;

	case ObjectTag::UI:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::UI), std::move(tmpObject) });
		break;

	case ObjectTag::PLAYER:
		{
			CObject* rawObj = PrefabManager::GetInstance().Instantiate("PlayerJSON", _ObjectName);
			if (!rawObj) rawObj = PrefabManager::GetInstance().InstantiateFromJSON("Assets/Prefabs/Player.json", _ObjectName);
			tmpObject = std::unique_ptr<CObject>(rawObj);
			returnObject = tmpObject.get();
			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::PLAYER), std::move(tmpObject) });
		}
		break;

	case ObjectTag::PLAYER_BULLET:
		{
			CObject* rawObj = PrefabManager::GetInstance().Instantiate("PlayerBulletJSON", _ObjectName);
			if (!rawObj) rawObj = PrefabManager::GetInstance().InstantiateFromJSON("Assets/Prefabs/PlayerBullet.json", _ObjectName);
			tmpObject = std::unique_ptr<CObject>(rawObj);
			returnObject = tmpObject.get();
			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::PLAYER_BULLET), std::move(tmpObject) });
		}
		break;

	case ObjectTag::ENEMY:
		{
			CObject* rawObj = PrefabManager::GetInstance().Instantiate("EnemyJSON", _ObjectName);
			if (!rawObj) rawObj = PrefabManager::GetInstance().InstantiateFromJSON("Assets/Prefabs/Enemy.json", _ObjectName);
			tmpObject = std::unique_ptr<CObject>(rawObj);
			returnObject = tmpObject.get();
			m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::ENEMY), std::move(tmpObject) });
		}
		break;

	case ObjectTag::ENEMY_BULLET:
		break;

	case ObjectTag::FIELD:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		tmpObject->AddComponent<FieldComponent>();
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::FIELD), std::move(tmpObject) });
		break;

	case ObjectTag::BILLBOARD:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::BILLBOARD), std::move(tmpObject) });
		break;

	case ObjectTag::EFFECT:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::EFFECT), std::move(tmpObject) });
		break;

	case ObjectTag::TEXT:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		if (auto info = tmpObject->GetComponent<CObjectInfo>())
		{
			info->SetObjectTag(ObjectTag::TEXT);
		}
		tmpObject->AddComponent<CTextRenderer>();
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::TEXT), std::move(tmpObject) });
		break;

	case ObjectTag::CAMERA:
		tmpObject = std::make_unique<Camera>(_ObjectName);
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::CAMERA), std::move(tmpObject) });
		break;

	case ObjectTag::FADE:
		break;

	case ObjectTag::MANAGER:
		tmpObject = std::make_unique<CObject>(_ObjectName);
		if (auto info = tmpObject->GetComponent<CObjectInfo>())
		{
			info->SetObjectTag(ObjectTag::MANAGER);
		}
		tmpObject->AddComponent<EnemyCounterComponent>();
		returnObject = tmpObject.get();
		m_pendingAddObjects.push_back({ static_cast<ObjectTag>(ObjectTag::MANAGER), std::move(tmpObject) });
		break;
	}

	if (returnObject)
	{
		returnObject->SetName(_ObjectName);
	}

	return returnObject;
}



//CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
//{
//	//Todo : Factory????
//
//	//????
//	//Map???????????????????
//	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
//	CObject* returnObject = nullptr;
//
//
//	switch (_Tag)
//	{
//	case ObjectTag::NONE:
//		tmpObject = std::make_unique<C3D_Object>("3DObject");
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//?z?????
//
//		break;
//
//	case ObjectTag::BACKGROUND:
//		tmpObject = std::make_unique<Skydome>(_TypeName);
//		returnObject = tmpObject.get();
//		vecObject[static_cast<int>(ObjectTag::BACKGROUND)].push_back(std::move(tmpObject));
//		break;
//	case ObjectTag::UI:
//		if (_TypeName == "TitleUI") {
//			tmpObject = std::make_unique<TitleUI>(_TypeName);
//		}
//		else {
//			tmpObject = std::make_unique<CUIObject>(_TypeName);
//		}
//		returnObject = tmpObject.get();
//		vecObject[static_cast<int>(ObjectTag::UI)].push_back(std::move(tmpObject));
//		break;
//	case ObjectTag::PLAYER:
//		tmpObject = std::make_unique<Player>("Player");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::PLAYER_BULLET:
//
//
//
//		tmpObject = std::make_unique<Bullet>("Bullet");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//?z?????
//		break;
//	case ObjectTag::ENEMY:
//		tmpObject = std::make_unique<Enemy>("Enemy");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::ENEMY_BULLET:
//		break;
//	case ObjectTag::FIELD:
//		//Floor
//
//		break;
//	case ObjectTag::BILLBOARD:
//		if (_TypeName == "RandomParticle")
//		{
//			tmpObject = std::make_unique<RandomParticle>("Particle");		//????
//			returnObject = tmpObject.get();							//???|?C???^???
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//			break;
//		}
//		else if (_TypeName == "Explosion")
//		{
//			tmpObject = std::make_unique<Explosion>("Explosion");		//????
//			returnObject = tmpObject.get();							//???|?C???^???
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//			break;
//		}
//
//
//		tmpObject = std::make_unique<BillBoard>("BillBoard");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::EFFECT:
//		tmpObject = std::make_unique<Explosion>("Explosion");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//?z?????
//		break;
//		break;
//	case ObjectTag::TEXT:
//		tmpObject = std::make_unique<TextObject>("TextObject1");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::CAMERA:
//		tmpObject = std::make_unique<Camera>("Camera");		//????
//		returnObject = tmpObject.get();							//???|?C???^???
//		vecObject[static_cast<int>(ObjectTag::CAMERA)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::FADE:
//		break;
//
//	}
//
//	//?z?????
//
//	return returnObject;
//}


