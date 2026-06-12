#include "SceneTest.h"
#include "D2DTextRenderer.h"
#include "3D_Object.h"

#include "Object.h"
#include "UIObject.h"

#include "Model.h"
#include "DX12Manager.h"

#include "ObjectManager.h"

CSceneTest::CSceneTest()
{
    //----- Player -----
    C3D_Object* player = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::PLAYER, "Player"));
    player->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });

    //------ Enemy -----
    C3D_Object* enemy = (C3D_Object*)(ObjectManager::GetInstance().Instantiate(Scene::ID::NONE, ObjectTag::ENEMY, "Enemy"));
    enemy->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });
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

    // 仮のテキスト描画処理
    D2DTextRenderer::GetInstance().DrawTextStr(L"Hello, DirectWrite & Direct2D!", 100.0f, 100.0f, 36.0f, D2D1::ColorF::LightGreen);
    D2DTextRenderer::GetInstance().DrawTextStr(L"This text is requested from CSceneTest::Draw()", 100.0f, 150.0f, 20.0f, D2D1::ColorF::White);
    D2DTextRenderer::GetInstance().DrawTextStr(L"Rendered on DX12 BackBuffer using D3D11on12 overlay", 100.0f, 180.0f, 18.0f, D2D1::ColorF::LightSteelBlue);
}
