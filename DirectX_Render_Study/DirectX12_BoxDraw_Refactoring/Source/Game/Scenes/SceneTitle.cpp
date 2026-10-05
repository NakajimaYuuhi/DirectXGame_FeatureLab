#include "SceneTitle.h"
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
#include "Source/UI/RectTransform.h"
#include "Source/Util/Tween.h"
#include "InspectorUI.h"
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

    // -------------------------------------------------------------
    // UI RectTransform & Tween Entrance Animations
    // -------------------------------------------------------------
    auto FindUIObject = [](const std::string& name) -> CObject* {
        const auto& allObjects = ObjectManager::GetInstance().GetObjectList();
        for (const auto& tagVec : allObjects)
        {
            for (const auto& obj : tagVec)
            {
                if (obj && obj->GetName() == name)
                {
                    return obj.get();
                }
            }
        }
        return nullptr;
    };

    // 1. Title Background: Fullscreen Stretch & Fade In
    CObject* bgObj = FindUIObject("TitleBG");
    if (bgObj)
    {
        CRectTransform* rect = bgObj->GetComponent<CRectTransform>();
        if (!rect) rect = bgObj->AddComponent<CRectTransform>();
        rect->SetAnchorPreset(AnchorPreset::StretchAll, true);
        rect->SetSizeDelta(0.0f, 0.0f);
        rect->SetScale(1.0f, 1.0f);

        if (auto sprite = bgObj->GetComponent<CSpriteRenderer>())
        {
            sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
            sprite->DOFade(1.0f, 0.8f)->SetEase(Ease::OutCubic);
        }
    }
    else
    {
        OutputDebugStringA("[SceneTitle] Warning: TitleBG object not found!\n");
    }

    // 2. Start Button: Center Anchored, Slide Up & Pop-up Scale
    CObject* startBtn = FindUIObject("StartButton");
    if (startBtn)
    {
        CRectTransform* rect = startBtn->GetComponent<CRectTransform>();
        if (!rect) rect = startBtn->AddComponent<CRectTransform>();
        rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
        rect->SetSizeDelta(400.0f, 100.0f);

        // Initial off-target state for entrance animation
        rect->SetAnchoredPosition(-20.0f, 260.0f);
        rect->SetScale(0.0f, 0.0f);

        // Entrance Tweens
        rect->DOAnchorPos({ -20.0f, 150.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.2f);
        rect->DOScale({ 1.0f, 1.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.2f);

        if (auto sprite = startBtn->GetComponent<CSpriteRenderer>())
        {
            sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
            sprite->DOFade(1.0f, 0.4f)->SetDelay(0.2f);
        }
    }
    else
    {
        OutputDebugStringA("[SceneTitle] Warning: StartButton object not found!\n");
    }

    // 3. Exit Button: Center Anchored, Slide Up & Pop-up Scale (Sequential)
    CObject* exitBtn = FindUIObject("ExitButton");
    if (exitBtn)
    {
        CRectTransform* rect = exitBtn->GetComponent<CRectTransform>();
        if (!rect) rect = exitBtn->AddComponent<CRectTransform>();
        rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
        rect->SetSizeDelta(400.0f, 100.0f);

        // Initial off-target state
        rect->SetAnchoredPosition(-20.0f, 390.0f);
        rect->SetScale(0.0f, 0.0f);

        // Entrance Tweens
        rect->DOAnchorPos({ -20.0f, 290.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.35f);
        rect->DOScale({ 1.0f, 1.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.35f);

        if (auto sprite = exitBtn->GetComponent<CSpriteRenderer>())
        {
            sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
            sprite->DOFade(0.4f, 0.4f)->SetDelay(0.35f);
        }
    }
    else
    {
        OutputDebugStringA("[SceneTitle] Warning: ExitButton object not found!\n");
    }

    // Ensure FirstSelected button is highlighted
    if (startBtn)
    {
        if (auto btn = startBtn->GetComponent<ButtonComponent>())
        {
            ButtonEventManager::GetInstance().SetSelectedGameObject(btn);
        }
    }

    // Save configured Title UI setup back to SceneTitle.json
    SceneSerializer::SaveScene("Assets/Scene/SceneTitle.json", Scenes::ID::TITLE);

    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice(); 
    UINT width = SCREEN_WIDTH;
    UINT height = SCREEN_HEIGHT;
    const float sceneClearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
    m_pOffscreenTexture = std::make_unique<RenderTexture>(pDevice, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, sceneClearColor);
    m_renderPipeline = std::make_unique<RenderPipeline>();
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(m_pOffscreenTexture.get()));
    m_renderPipeline->AddPass(std::make_unique<PostProcessPass>(m_pOffscreenTexture.get()));
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