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
#include "PostProcessPass.h" // ???

//???(width,height?èÔ)
#include "BasicSettings.h"

#include <memory>

SceneClear::SceneClear()
    :CScene(Scenes::ID::Clear)
{
}

SceneClear::~SceneClear() = default;

void SceneClear::Init()
{

    // ?{?^?????????
    ButtonEventManager::GetInstance();


    // ===== ?I?u?W?F?N?g?????

    // 1.Camera ???????
    ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera", "Camera");

    CUIObject* titleUI = (CUIObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "CUIObject", "ClearBG"));
    titleUI->SetTexture(L"Assets/Texture/T_Clear.png");
    titleUI->SetPosition(0.0f, 0.0f);
    titleUI->SetSize(1920.0f, 1080.0f);

    // --- Button Creation
    CUIButton* titleButton = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "CUIButton", "RetryButton"));
    titleButton->SetTexture(L"Assets/Texture/T_Retry.png");
    titleButton->SetPosition(740.0f, 650.0f);
    titleButton->SetSize(400.0f, 100.0f);
    titleButton->SetAction(ButtonAction::ChangeScene_Test);

    CUIButton* titleButton2 = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "CUIButton", "ToTitleButton"));
    titleButton2->SetTexture(L"Assets/Texture/T_ToTitle.png");
    titleButton2->SetPosition(735.0f, 800.0f);
    titleButton2->SetSize(400.0f, 100.0f);
    titleButton2->SetAction(ButtonAction::ChangeScene_Title);

    // Navigation setup
    titleButton->SetNavigation(titleButton2, titleButton2, nullptr, nullptr);
    titleButton2->SetNavigation(titleButton, titleButton, nullptr, nullptr);

    ObjectManager::GetInstance().Init(Scenes::ID::NONE);

    // --- Select registration
    ButtonEventManager::GetInstance().SetFirstSelectedName("RetryButton");
    ButtonEventManager::GetInstance().SetSelectedGameObject((CUIButton*)titleButton);


    // ----- ?p?C?v???C????? -----
    m_renderPipeline = std::make_unique<RenderPipeline>();
    // 1. ?I?t?X?N???[???e?N?X?`???????
    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice();
    UINT width = SCREEN_WIDTH; // ????
    UINT height = SCREEN_HEIGHT; // ??????
    m_pOffscreenTexture = std::make_unique<RenderTexture>(pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
    // 2. ?p?C?v???C?????????p?X??o?^
    m_renderPipeline = std::make_unique<RenderPipeline>();

    // ForwardRenderPass ????????????o?b?N?o?b?t?@??????????????????
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(nullptr));

    
    // 3. ?p?C?v???C?????S?p?X??????? (PSO?????????????)
    m_renderPipeline->Init(pDevice);
}

void SceneClear::Update()
{

    ButtonEventManager::GetInstance().Update();

    //?L?[?????C?x???g??????
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
    ctx.deltaTime = 1.0f / 60.0f; // ????? deltaTime ??u???????????????
    ctx.backBufferRTV = DX12Manager::GetInstance().GetCurrentBackBufferRTV();
    ctx.mainDSV = DX12Manager::GetInstance().GetMainDSV();
    ctx.screenWidth = SCREEN_WIDTH;
    ctx.screenHeight = SCREEN_HEIGHT;
    //ctx.pCamera       = ObjectManager::GetInstance().GetCamera(); // ?J????????Z?b?g
   // 2. ?p?C?v???C??????s?I
    m_renderPipeline->Execute(ctx);
}
