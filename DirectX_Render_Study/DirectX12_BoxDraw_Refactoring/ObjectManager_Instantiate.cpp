//ObjectManager_Getter_Setter.cpp

//Instantiateの処理

//====== インクルード =====

//ヘッダ
#include "ObjectManager.h"

//オブジェクト

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

// --Manager
#include "EnemyCounter.h"


//===== メソッド定義 =====
CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
{
	//Todo : Factoryを作る

	//生成
	//Mapとかできれいに分岐させたい
	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
	CObject* returnObject = nullptr;


	switch (_Tag)
	{
	case ObjectTag::NONE:
		tmpObject = std::make_unique<C3D_Object>("3DObject");
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//配列に追加

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
		else {
			tmpObject = std::make_unique<CUIObject>(_TypeName);
		}
		returnObject = tmpObject.get();
		vecObject[static_cast<int>(ObjectTag::UI)].push_back(std::move(tmpObject));
		break;
	case ObjectTag::PLAYER:
		tmpObject = std::make_unique<Player>("Player");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//配列に追加
		break;
	case ObjectTag::PLAYER_BULLET:



		tmpObject = std::make_unique<Bullet>("Bullet");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//配列に追加
		break;
	case ObjectTag::ENEMY:
		tmpObject = std::make_unique<Enemy>("Enemy");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//配列に追加
		break;
	case ObjectTag::ENEMY_BULLET:
		break;
	case ObjectTag::FIELD:
		//Floor

		break;
	case ObjectTag::BILLBOARD:
		if (_TypeName == "RandomParticle")
		{
			tmpObject = std::make_unique<RandomParticle>("Particle");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
			break;
		}
		else if (_TypeName == "Explosion")
		{
			tmpObject = std::make_unique<Explosion>("Explosion");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
			break;
		}


		tmpObject = std::make_unique<BillBoard>("BillBoard");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
		break;
	case ObjectTag::EFFECT:
		tmpObject = std::make_unique<Explosion>("Explosion");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//配列に追加
		break;
		break;
	case ObjectTag::TEXT:
		if (_TypeName == "TextObject1")
		{
			tmpObject = std::make_unique<TextObject>("TextObject1");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//配列に追加
			break;
		}
		if (_TypeName == "EnemyCount")
		{
			tmpObject = std::make_unique<EnemyCount>("EnemyCount");		//生成
			returnObject = tmpObject.get();							//生ポインタ取得
			vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//配列に追加
			break;
		}
		break;
	case ObjectTag::CAMERA:
		tmpObject = std::make_unique<Camera>("Camera");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::CAMERA)].push_back(std::move(tmpObject));				//配列に追加
		break;
	case ObjectTag::FADE:
		break;

	case ObjectTag::MANAGER:
		tmpObject = std::make_unique<EnemyCounter>("EnemyCounter");		//生成
		returnObject = tmpObject.get();							//生ポインタ取得
		vecObject[static_cast<int>(ObjectTag::MANAGER)].push_back(std::move(tmpObject));				//配列に追加
		
		break;

	}

	//配列に追加

	return returnObject;
}



//CObject* ObjectManager::Instantiate(Scenes::ID _SceneID, ObjectTag _Tag, std::string _TypeName)
//{
//	//Todo : Factoryを作る
//
//	//生成
//	//Mapとかできれいに分岐させたい
//	std::unique_ptr<CObject> tmpObject = std::unique_ptr<CObject>(nullptr);
//	CObject* returnObject = nullptr;
//
//
//	switch (_Tag)
//	{
//	case ObjectTag::NONE:
//		tmpObject = std::make_unique<C3D_Object>("3DObject");
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::FIELD)].push_back(std::move(tmpObject));				//配列に追加
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
//		tmpObject = std::make_unique<Player>("Player");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::PLAYER)].push_back(std::move(tmpObject));				//配列に追加
//		break;
//	case ObjectTag::PLAYER_BULLET:
//
//
//
//		tmpObject = std::make_unique<Bullet>("Bullet");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::PLAYER_BULLET)].push_back(std::move(tmpObject));		//配列に追加
//		break;
//	case ObjectTag::ENEMY:
//		tmpObject = std::make_unique<Enemy>("Enemy");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::ENEMY)].push_back(std::move(tmpObject));				//配列に追加
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
//			tmpObject = std::make_unique<RandomParticle>("Particle");		//生成
//			returnObject = tmpObject.get();							//生ポインタ取得
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
//			break;
//		}
//		else if (_TypeName == "Explosion")
//		{
//			tmpObject = std::make_unique<Explosion>("Explosion");		//生成
//			returnObject = tmpObject.get();							//生ポインタ取得
//			vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
//			break;
//		}
//
//
//		tmpObject = std::make_unique<BillBoard>("BillBoard");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::BILLBOARD)].push_back(std::move(tmpObject));				//配列に追加
//		break;
//	case ObjectTag::EFFECT:
//		tmpObject = std::make_unique<Explosion>("Explosion");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::EFFECT)].push_back(std::move(tmpObject));				//配列に追加
//		break;
//		break;
//	case ObjectTag::TEXT:
//		tmpObject = std::make_unique<TextObject>("TextObject1");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::TEXT)].push_back(std::move(tmpObject));				//配列に追加
//		break;
//	case ObjectTag::CAMERA:
//		tmpObject = std::make_unique<Camera>("Camera");		//生成
//		returnObject = tmpObject.get();							//生ポインタ取得
//		vecObject[static_cast<int>(ObjectTag::CAMERA)].push_back(std::move(tmpObject));				//配列に追加
//		break;
//	case ObjectTag::FADE:
//		break;
//
//	}
//
//	//配列に追加
//
//	return returnObject;
//}