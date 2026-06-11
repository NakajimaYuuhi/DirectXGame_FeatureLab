#include "SceneTest.h"
#include "3D_Object.h"

#include "Object.h"
#include "UIObject.h"

#include "Model.h"

#include "ObjectManager.h"

CSceneTest::CSceneTest()
{

    //----- Player -----
    //¡‚ÌŠ‰½‚à–³‚µ
    C3D_Object* player = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::PLAYER, "Player"));

    //Žè‘O‚ðˆê’UŒü‚¢‚Ä‚à‚ç‚¤
    player->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });


    //------ Enemy -----
    C3D_Object* enemy = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::ENEMY, "Enemy"));

    //Žè‘O‚ðˆê’UŒü‚¢‚Ä‚à‚ç‚¤
    enemy->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });

    // --- UI ---
    auto ui = (CUIObject*)ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::UI, "UIObject");
    ui->SetTexture(L"Assets/Texture/Sample1.jpg");
    ui->SetPosition(0.0f, 0.0f);
    ui->SetSize(200.0f, 200.0f);


}



CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{

    ObjectManager::GetInstance().Init(Scene::ID::NONE);


}

void CSceneTest::Update() 
{
	ObjectManager::GetInstance().Update(Scene::ID::NONE);

}


void CSceneTest::Draw() 
{

	ObjectManager::GetInstance().Draw(Scene::ID::NONE);
}

