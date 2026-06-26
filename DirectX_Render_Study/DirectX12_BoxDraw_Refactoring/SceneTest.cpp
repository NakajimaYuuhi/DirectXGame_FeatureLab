#include "SceneTest.h"
#include "TextObject.h"
#include "3D_Object.h"

#include "Object.h"
#include "UIObject.h"

#include "Model.h"
#include "DX12Manager.h"

#include "ObjectManager.h"

#include "InputManager.h"

#include "EventManager.h"

#include "EventData_NextScene.h"

#include "EnemyCounter.h"

CSceneTest::CSceneTest()
    :CScene(Scenes::ID::TEST)
{

}

CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{

    ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");

    //----- Player -----
    C3D_Object* player = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::PLAYER, "Player"));
    player->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

    //------ Enemy -----
    C3D_Object* enemy = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::ENEMY, "Enemy"));
    enemy->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });

    //------ Skydome -----
    C3D_Object* skydome = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BACKGROUND, "Skydome"));
    skydome->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });


    //------ TextObject (D2D/DirectWrite Overlay) -----
    TextObject* textObj1 = (TextObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::TEXT, "TextObject1"));
    textObj1->SetText(L"DirectWrite & Direct2D Text Rendering System (Game Object!)");
    textObj1->SetPosition(50.0f, 50.0f);
    textObj1->SetFontSize(36.0f);
    textObj1->SetColor(D2D1::ColorF::Cyan);

    TextObject* textObj2 = (TextObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::TEXT, "TextObject1"));
    textObj2->SetText(L"Drawing text via CTextRenderer Component attached to CTextObject");
    textObj2->SetPosition(50.0f, 100.0f);
    textObj2->SetFontSize(24.0f);
    textObj2->SetColor(D2D1::ColorF::Yellow);

    TextObject* textObj3 = (TextObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::TEXT, "TextObject1"));
    textObj3->SetText(L"Scene: TestScene (D2D Text Game Objects)");
    textObj3->SetPosition(50.0f, 140.0f);
    textObj3->SetFontSize(18.0f);
    textObj3->SetColor(D2D1::ColorF::LightPink);

    //----- EnemyCounter -----
    EnemyCounter* enemyCounter = (EnemyCounter*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::MANAGER, "EnemyCounter"));


    ObjectManager::GetInstance().Init(Scenes::ID::NONE);
}

void CSceneTest::Update() 
{
    if (CInputManager::GetInstance().IsKeyTrigger('P'))
    {
        Event event;
        EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::TITLE);

        event.SetEventData(eventData_NextScene);

        event.SetEventID(Events::ID::ChangeScene);

        EventManager::GetInstance().AddEvent(event);
    }


	ObjectManager::GetInstance().Update(Scenes::ID::NONE);
}

void CSceneTest::Draw() 
{
	ObjectManager::GetInstance().Draw(Scenes::ID::NONE);
}
