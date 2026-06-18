#include "SceneTitle.h"
#include "TextObject.h"
#include "3D_Object.h"

#include "Object.h"
#include "UIObject.h"

#include "Model.h"
#include "DX12Manager.h"

#include "ObjectManager.h"

#include "InputManager.h"

SceneTitle::SceneTitle()
    :CScene(Scenes::ID::TEST)
{



    //------ TextObject (D2D/DirectWrite Overlay) -----
    TextObject* textObj1 = (TextObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::TEXT, "TextObject1"));
    textObj1->SetText(L"Title");
    textObj1->SetPosition(50.0f, 50.0f);
    textObj1->SetFontSize(36.0f);
    textObj1->SetColor(D2D1::ColorF::Cyan);


}

SceneTitle::~SceneTitle() = default;

void SceneTitle::Init()
{
    ObjectManager::GetInstance().Init(Scenes::ID::NONE);
}

void SceneTitle::Update()
{
    ObjectManager::GetInstance().Update(Scenes::ID::NONE);


}

void SceneTitle::Draw()
{
    ObjectManager::GetInstance().Draw(Scenes::ID::NONE);
}
