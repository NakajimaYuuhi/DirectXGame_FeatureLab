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
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include "ForwardRenderPass.h"
#include "PostProcessPass.h"
#include "BasicSettings.h"
#include <memory>

SceneTitle::SceneTitle()
    :CScene(Scenes::ID::TITLE)
{
}

SceneTitle::~SceneTitle() = default;

void SceneTitle::Init()
{
    ButtonEventManager::GetInstance();

    if (!SceneSerializer::LoadSceneOrDefault("Assets/Scene/SceneTitle.json", Scenes::ID::TITLE))
    {
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");

        CUIObject* titleUI = (CUIObject*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "TitleUI"));
        titleUI->SetTexture(L"Assets/Texture/T_TitleBG.png");
        titleUI->SetPosition(0.0f, 0.0f);
        titleUI->SetSize(1920.0f, 1080.0f);

        CUIButton* titleButton = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
        titleButton->SetTexture(L"Assets/Texture/T_GameStart.png");
        titleButton->SetPosition(740.0f, 650.0f);
        titleButton->SetSize(400.0f, 100.0f);
        titleButton->SetAction(ButtonAction::ChangeScene_Test);

        CUIButton* titleButton2 = (CUIButton*)(ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton"));
        titleButton2->SetTexture(L"Assets/Texture/T_Exit.png");
        titleButton2->SetPosition(735.0f, 800.0f);
        titleButton2->SetSize(400.0f, 100.0f);
        titleButton2->SetAction(ButtonAction::ExitGame);

        titleButton->SetNavigation(titleButton2, titleButton2, nullptr, nullptr);
        titleButton2->SetNavigation(titleButton, titleButton, nullptr, nullptr);

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);
        ButtonEventManager::GetInstance().SetFirstSelectedName("UIButton");
        ButtonEventManager::GetInstance().SetSelectedGameObject((CUIButton*)titleButton);

        SceneSerializer::SaveScene("Assets/Scene/SceneTitle.json", Scenes::ID::TITLE);
    }

    m_renderPipeline = std::make_unique<RenderPipeline>();
    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice(); 
    UINT width = SCREEN_WIDTH;
    UINT height = SCREEN_HEIGHT;
    m_pOffscreenTexture = std::make_unique<RenderTexture>(pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
    m_renderPipeline = std::make_unique<RenderPipeline>();
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(nullptr));
    m_renderPipeline->Init(pDevice);
}

void SceneTitle::Update()
{
    ButtonEventManager::GetInstance().Update();

    if (CInputManager::GetInstance().IsKeyTrigger('P'))
    {
        Event event;
        EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::TEST);
        event.SetEventData(eventData_NextScene);
        event.SetEventID(Events::ID::ChangeScene);
        EventManager::GetInstance().AddEvent(event);
    }

    if (CInputManager::GetInstance().IsKeyTrigger('O'))
    {
        Event event;
        EventData_NextScene* eventData_NextScene = new EventData_NextScene(Scenes::ID::Clear);
        event.SetEventData(eventData_NextScene);
        event.SetEventID(Events::ID::ChangeScene);
        EventManager::GetInstance().AddEvent(event);
    }

    ObjectManager::GetInstance().Update(Scenes::ID::NONE);
}

void SceneTitle::Draw()
{
    RenderContext ctx;
    ctx.cmdList       = DX12Manager::GetInstance().GetCommandList();
    ctx.sceneID       = Scenes::ID::NONE;
    ctx.deltaTime     = 1.0f / 60.0f;
    ctx.backBufferRTV = DX12Manager::GetInstance().GetCurrentBackBufferRTV();
    ctx.mainDSV       = DX12Manager::GetInstance().GetMainDSV();
    ctx.screenWidth   = SCREEN_WIDTH;
    ctx.screenHeight  = SCREEN_HEIGHT;

    m_renderPipeline->Execute(ctx);
}
