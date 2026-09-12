//ObjectManager_Getter_Setter.cpp

//Instantiate?????

//====== ?C???N???[?h =====

//?w?b?_
#include "ObjectManager.h"

//?I?u?W?F?N?g

// --3D
#include "3D_Object.h"

//character
#include "Player.h"
#include "Enemy.h"

//bullet
#include "Bullet.h"

//billboard
#include "BillBoard.h"

//effect
#include "RandomParticle.h"
#include "Explosion.h"

//skydome
#include "Skydome.h"

//camera
#include "Camera.h"


// --2D
#include "UIObject.h"
#include "TextObject.h"
#include "EnemyCount.h"
#include "TitleUI.h"
#include "CUIButton.h"

// --Manager
#include "EnemyCounter.h"


//===== ???\?b?h??` =====
CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
{
	//Todo : Factory????

	//????
	//Map???????????????????
	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
	CObject* returnObject = nullptr;


	switch (_Tag)
	{
	case ObjectTag::NONE:
		tmpObject = std::make_unique<C3D_Object>("3DObject");
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//?z?????

		break;

	case ObjectTag::BACKGROUND:
		tmpObject = std::make_unique<Skydome>(_TypeName);
		returnObject = tmpObject.get();
		vecObject[static_cast<int>(ObjectTag::BACKGROUND)].push_back(std::move(tmpObject));
		break;
	case ObjectTag::UI:
		if (_TypeName == "TitleUI") {
			tmpObject = std::make_unique<TitleUI>(_TypeName);
		}
		else if (_TypeName == "UIButton") {
			tmpObject = std::make_unique<CUIButton>(_TypeName);
		}
		returnObject = tmpObject.get();
		vecObject[static_cast<int>(ObjectTag::UI)].push_back(std::move(tmpObject));
		break;
	case ObjectTag::PLAYER:
		tmpObject = std::make_unique<Player>("Player");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//?z?????
		break;
	case ObjectTag::PLAYER_BULLET:



		tmpObject = std::make_unique<Bullet>("Bullet");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//?z?????
		break;
	case ObjectTag::ENEMY:
		tmpObject = std::make_unique<Enemy>("Enemy");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//?z?????
		break;
	case ObjectTag::ENEMY_BULLET:
		break;
	case ObjectTag::FIELD:
		//Floor

		break;
	case ObjectTag::BILLBOARD:
		if (_TypeName == "RandomParticle")
		{
			tmpObject = std::make_unique<RandomParticle>("Particle");		//????
			returnObject = tmpObject.get();							//???|?C???^?èÔ
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
			break;
		}
		else if (_TypeName == "Explosion")
		{
			tmpObject = std::make_unique<Explosion>("Explosion");		//????
			returnObject = tmpObject.get();							//???|?C???^?èÔ
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
			break;
		}


		tmpObject = std::make_unique<BillBoard>("BillBoard");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
		break;
	case ObjectTag::EFFECT:
		tmpObject = std::make_unique<Explosion>("Explosion");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//?z?????
		break;
		break;
	case ObjectTag::TEXT:
		if (_TypeName == "TextObject1")
		{
			tmpObject = std::make_unique<TextObject>("TextObject1");		//????
			returnObject = tmpObject.get();							//???|?C???^?èÔ
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//?z?????
			break;
		}
		if (_TypeName == "EnemyCount")
		{
			tmpObject = std::make_unique<EnemyCount>("EnemyCount");		//????
			returnObject = tmpObject.get();							//???|?C???^?èÔ
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//?z?????
			break;
		}
		break;
	case ObjectTag::CAMERA:
		tmpObject = std::make_unique<Camera>("Camera");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::CAMERA)].push_back(std::move(tmpObject));				//?z?????
		break;
	case ObjectTag::FADE:
		break;

	case ObjectTag::MANAGER:
		tmpObject = std::make_unique<EnemyCounter>("EnemyCounter");		//????
		returnObject = tmpObject.get();							//???|?C???^?èÔ
		vecObject[static_cast<int>(ObjectTag::MANAGER)].push_back(std::move(tmpObject));				//?z?????
		
		break;

	}

	//?z?????

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
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
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
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
//		vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::PLAYER_BULLET:
//
//
//
//		tmpObject = std::make_unique<Bullet>("Bullet");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
//		vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//?z?????
//		break;
//	case ObjectTag::ENEMY:
//		tmpObject = std::make_unique<Enemy>("Enemy");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
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
//			returnObject = tmpObject.get();							//???|?C???^?èÔ
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//			break;
//		}
//		else if (_TypeName == "Explosion")
//		{
//			tmpObject = std::make_unique<Explosion>("Explosion");		//????
//			returnObject = tmpObject.get();							//???|?C???^?èÔ
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//			break;
//		}
//
//
//		tmpObject = std::make_unique<BillBoard>("BillBoard");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
//		vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::EFFECT:
//		tmpObject = std::make_unique<Explosion>("Explosion");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
//		vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//?z?????
//		break;
//		break;
//	case ObjectTag::TEXT:
//		tmpObject = std::make_unique<TextObject>("TextObject1");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
//		vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//?z?????
//		break;
//	case ObjectTag::CAMERA:
//		tmpObject = std::make_unique<Camera>("Camera");		//????
//		returnObject = tmpObject.get();							//???|?C???^?èÔ
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
