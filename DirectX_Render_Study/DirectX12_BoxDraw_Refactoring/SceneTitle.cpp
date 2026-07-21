#include "SceneTitle.h"
#include "TextObject.h"
#include "3D_Object.h"

#include "Object.h"
#include "UIObject.h"
#include "CUIButton.h"

#include "Model.h"
#include "DX12Manager.h"

#include "ObjectManager.h"

#include "InputManager.h"

#include "EventManager.h"

#include "EventData_NextScene.h"

#include "ObjectTag.h"

#include "ButtonEventManager.h"


//
#include "ForwardRenderPass.h"

SceneTitle::SceneTitle()
    :CScene(Scenes::ID::TITLE)
{






}

SceneTitle::~SceneTitle() = default;

void SceneTitle::Init()
{
    ButtonEventManager::GetInstance();

    //Cameraの定義
    ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA,"Camera");

    //------ TextObject (D2D/DirectWrite Overlay) -----
    TextObject* textObj1 = (TextObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::TEXT, "TextObject1"));
    textObj1->SetText(L"Title");
    textObj1->SetPosition(50.0f, 50.0f);
    textObj1->SetFontSize(36.0f);
    textObj1->SetColor(D2D1::ColorF::Cyan);


    // TitleUI
    CUIObject* titleUI = (CUIObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "TitleUI"));
    titleUI->SetTexture(L"Assets/Texture/TmpActionGameTItleImage.png");
    titleUI->SetPosition(0.0f, 0.0f);
    titleUI->SetSize(1920.0f, 1080.0f);

    CUIButton* titleButton = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
    titleButton->SetTexture(L"Assets/Texture/TmpActionGameTItleImage.png");
    titleButton->SetPosition(0.0f, 0.0f);
    titleButton->SetSize(500.0f, 500.0f);

    CUIButton* titleButton2 = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
    titleButton2->SetTexture(L"Assets/Texture/TmpActionGameTItleImage.png");
    titleButton2->SetPosition(800.0f, 0.0f);
    titleButton2->SetSize(500.0f, 500.0f);

    titleButton->SetNavigation(titleButton2, titleButton2, nullptr, nullptr);
    titleButton2->SetNavigation(titleButton, titleButton, nullptr, nullptr);

    ObjectManager::GetInstance().Init(Scenes::ID::NONE);
    
    ButtonEventManager::GetInstance().SetSelectedGameObject((CUIButton*)titleButton);

    // 1. パスの構築と登録
// ※今回は Phase1 なのでオフスクリーンテクスチャは nullptr を渡すか、
// バックバッファ直書き用の ForwardRenderPass を作って登録します。
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>());
}

void SceneTitle::Update()
{

    ButtonEventManager::GetInstance().Update();

    //キー入力でイベントを入れる
    if (CInputManager::GetInstance().IsKeyTrigger('P'))
    {
        Event event;
        EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::TEST);

        event.SetEventData(eventData_NextScene);

        event.SetEventID(Events::ID::ChangeScene);

        EventManager::GetInstance().AddEvent(event);
    }


    ObjectManager::GetInstance().Update(Scenes::ID::NONE);


}

void SceneTitle::Draw()
{
    //ObjectManager::GetInstance().Draw(Scenes::ID::NONE);
    RenderContext ctx;
     ctx.cmdList       = DX12Manager::GetInstance().GetCommandList();
     ctx.sceneID       = Scenes::ID::NONE;
     ctx.deltaTime     = 1.0f / 60.0f; // 実際の deltaTime に置き換えてください
     ctx.backBufferRTV = backBufferRTV;
     ctx.mainDSV       = mainDSV;
     ctx.screenWidth   = width;
     ctx.screenHeight  = height;
     ctx.pCamera       = ObjectManager::GetInstance().GetCamera(); // カメラ情報のセット
    // 2. パイプラインの実行！
    m_renderPipeline->Execute(ctx);
}
