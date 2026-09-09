#include "SceneClear.h"
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
#include "PostProcessPass.h" // 追加

//設定(width,height取得)
#include "BasicSettings.h"

#include <memory>

SceneClear::SceneClear()
    :CScene(Scenes::ID::Clear)
{
}

SceneClear::~SceneClear() = default;

void SceneClear::Init()
{

    // ボタンの初期化
    ButtonEventManager::GetInstance();


    // ===== オブジェクトの生成

    // 1.Camera これは絶対
    ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");



    // 背景
    // 背景だけど、BackGroundにすると、スカイボックスがそのまま出ちゃう
    // →UIだしそのままUIで出す
    CUIObject* titleUI = (CUIObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "TitleUI"));
    titleUI->SetTexture(L"Assets/Texture/T_Clear.png");
    titleUI->SetPosition(0.0f, 0.0f);
    titleUI->SetSize(1920.0f, 1080.0f);

    // --- ボタンの作成
    CUIButton* titleButton = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
    titleButton->SetTexture(L"Assets/Texture/T_Retry.png");
    titleButton->SetPosition(740.0f, 650.0f);
    titleButton->SetSize(400.0f, 100.0f);

    CUIButton* titleButton2 = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
    titleButton2->SetTexture(L"Assets/Texture/T_ToTitle.png");
    titleButton2->SetPosition(735.0f, 800.0f);
    titleButton2->SetSize(400.0f, 100.0f);

    // 遷移先
    titleButton->SetNavigation(titleButton2, titleButton2, nullptr, nullptr);
    titleButton2->SetNavigation(titleButton, titleButton, nullptr, nullptr);

    // ボタンの関数登録
    titleButton->SetOnClickCallback(
        [this]() {
            Event event;
            // ここで直接シーンID（例: TITLE）を指定する
            EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::TEST);

            event.SetEventData(eventData_NextScene);
            event.SetEventID(Events::ID::ChangeScene);

            EventManager::GetInstance().AddEvent(event);
        }
    );

    titleButton2->SetOnClickCallback(
        [this]() {
            Event event;
            // ここで直接シーンID（例: TITLE）を指定する
            EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::TITLE);

            event.SetEventData(eventData_NextScene);
            event.SetEventID(Events::ID::ChangeScene);

            EventManager::GetInstance().AddEvent(event);
        }
    );



    ObjectManager::GetInstance().Init(Scenes::ID::NONE);

    // --- Select状態に登録
    ButtonEventManager::GetInstance().SetSelectedGameObject((CUIButton*)titleButton);


    // ----- パイプラインの作成 -----
    m_renderPipeline = std::make_unique<RenderPipeline>();
    // 1. オフスクリーンテクスチャの生成
    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice();
    UINT width = SCREEN_WIDTH; // 画面幅
    UINT height = SCREEN_HEIGHT; // 画面高さ
    m_pOffscreenTexture = std::make_unique<RenderTexture>(pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
    // 2. パイプラインの生成とパスの登録
    m_renderPipeline = std::make_unique<RenderPipeline>();

    // ForwardRenderPass 何も設定しなければバックバッファに変えない実装になってる
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(nullptr));

    
    // 3. パイプライン内の全パスを初期化 (PSOの生成などが走る)
    m_renderPipeline->Init(pDevice);
}

void SceneClear::Update()
{

    ButtonEventManager::GetInstance().Update();

    //キー入力でイベントを入れる
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

void SceneClear::Draw()
{
    //ObjectManager::GetInstance().Draw(Scenes::ID::NONE);
    RenderContext ctx;
    ctx.cmdList = DX12Manager::GetInstance().GetCommandList();
    ctx.sceneID = Scenes::ID::NONE;
    ctx.deltaTime = 1.0f / 60.0f; // 実際の deltaTime に置き換えてください
    ctx.backBufferRTV = DX12Manager::GetInstance().GetCurrentBackBufferRTV();
    ctx.mainDSV = DX12Manager::GetInstance().GetMainDSV();
    ctx.screenWidth = SCREEN_WIDTH;
    ctx.screenHeight = SCREEN_HEIGHT;
    //ctx.pCamera       = ObjectManager::GetInstance().GetCamera(); // カメラ情報のセット
   // 2. パイプラインの実行！
    m_renderPipeline->Execute(ctx);
}
