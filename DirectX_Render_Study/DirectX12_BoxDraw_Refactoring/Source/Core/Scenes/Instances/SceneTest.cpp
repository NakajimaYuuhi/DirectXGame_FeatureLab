#include "SceneTest.h"
#include "TextObject.h"
#include "3D_Object.h"
#include "Object.h"
#include "UIObject.h"
#include "Model.h"
#include "DX12Manager.h"
#include "Player.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "EventData_NextScene.h"
#include "EnemyCounter.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"

CSceneTest::CSceneTest()
    :CScene(Scenes::ID::TEST)
{
}

CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{
    if (!SceneSerializer::LoadSceneOrDefault("Assets/Scene/SceneTest.json", Scenes::ID::TEST))
    {
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");

        C3D_Object* player = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::PLAYER, "Player"));
        player->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

        C3D_Object* enemy = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::ENEMY, "Enemy"));
        enemy->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });

        C3D_Object* skydome = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BACKGROUND, "Skydome"));
        skydome->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });

        EnemyCounter* enemyCounter = (EnemyCounter*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::MANAGER, "EnemyCounter"));

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);

        SceneSerializer::SaveScene("Assets/Scene/SceneTest.json", Scenes::ID::TEST);
    }
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
