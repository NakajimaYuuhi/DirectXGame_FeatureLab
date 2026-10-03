#include "SceneTitle.h"
#include "3D_Object.h"
#include "Object.h"
#include "SpriteRenderer.h"
#include "ButtonComponent.h"
#include "ButtonAction.h"
#include "Transform.h"
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
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera", "Camera");

        CObject* titleUI = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIImage", "TitleBG");
        if (titleUI)
        {
            if (auto sprite = titleUI->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_TitleBG.png");
                sprite->SetSize(1920.0f, 1080.0f);
            }
        }

        CObject* titleButton = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "StartButton");
        if (titleButton)
        {
            if (auto sprite = titleButton->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_GameStart.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto transform = titleButton->GetComponent<CTransform>())
            {
                transform->SetPos({ 740.0f, 650.0f, 0.0f });
            }
            if (auto btnComp = titleButton->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ChangeScene_Test);
            }
        }

        CObject* titleButton2 = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "ExitButton");
        if (titleButton2)
        {
            if (auto sprite = titleButton2->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_Exit.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto transform = titleButton2->GetComponent<CTransform>())
            {
                transform->SetPos({ 735.0f, 800.0f, 0.0f });
            }
            if (auto btnComp = titleButton2->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ExitGame);
            }
        }

        if (titleButton && titleButton2)
        {
            ButtonComponent* b1 = titleButton->GetComponent<ButtonComponent>();
            ButtonComponent* b2 = titleButton2->GetComponent<ButtonComponent>();
            if (b1 && b2)
            {
                b1->SetNavigationNames("ExitButton", "ExitButton", "", "");
                b2->SetNavigationNames("StartButton", "StartButton", "", "");
            }
        }

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);
        ButtonEventManager::GetInstance().SetFirstSelectedName("StartButton");
        if (titleButton)
        {
            ButtonEventManager::GetInstance().SetSelectedGameObject(titleButton->GetComponent<ButtonComponent>());
        }

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
