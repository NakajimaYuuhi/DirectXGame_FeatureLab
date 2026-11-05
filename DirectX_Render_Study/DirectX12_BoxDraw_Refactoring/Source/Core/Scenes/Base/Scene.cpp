#include "Scene.h"
#include "Object.h"
#include "Model.h"
#include "SpriteRenderer.h"
#include "ButtonComponent.h"
#include "ButtonAction.h"
#include "Transform.h"
#include "DX12Manager.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "EventData_NextScene.h"
#include "ObjectTag.h"
#include "ButtonEventManager.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include "RenderPipeline.h"
#include "RenderTexture.h"
#include "ShadowMapPass.h"
#include "ForwardRenderPass.h"
#include "PostProcessPass.h"
#include "SampleComputePass.h"
#include "GPUParticlePass.h"
#include "TimeManager.h"
#include "BasicSettings.h"
#include "Source/UI/RectTransform.h"
#include "Source/Util/Tween.h"
#include "LightComponent.h"
#include "FieldComponent.h"
#include <filesystem>

namespace fs = std::filesystem;

static std::string DefaultPathForLegacyID(Scenes::ID sceneId)
{
    switch (sceneId)
    {
    case Scenes::ID::TITLE:  return "Assets/Scene/SceneTitle.json";
    case Scenes::ID::TEST:   return "Assets/Scene/SceneTest.json";
    case Scenes::ID::Clear:  return "Assets/Scene/SceneClear.json";
    case Scenes::ID::Failed: return "Assets/Scene/SceneFailed.json";
    default:                 return "";
    }
}

static Scenes::ID InferLegacyIDFromPath(const std::string& jsonPath)
{
    std::string stem = fs::path(jsonPath).stem().string();
    if (stem == "SceneTitle")  return Scenes::ID::TITLE;
    if (stem == "SceneClear")  return Scenes::ID::Clear;
    if (stem == "SceneFailed") return Scenes::ID::Failed;
    if (stem == "SceneTest")   return Scenes::ID::TEST;
    return Scenes::ID::TEST;
}

CScene::CScene(Scenes::ID _id)
    : id(_id)
{
    m_jsonPath = DefaultPathForLegacyID(_id);
    if (!m_jsonPath.empty())
    {
        m_Name = fs::path(m_jsonPath).stem().string();
    }
}

CScene::CScene(const std::string& jsonPath, Scenes::ID _id)
    : m_jsonPath(jsonPath), id(_id)
{
    if (!m_jsonPath.empty())
    {
        m_Name = fs::path(m_jsonPath).stem().string();
    }
    if (id == Scenes::ID::NONE && !m_jsonPath.empty())
    {
        id = InferLegacyIDFromPath(m_jsonPath);
    }
}

CScene::~CScene() = default;

void CScene::SetJsonPath(const std::string& path)
{
    m_jsonPath = path;
    if (!m_jsonPath.empty())
    {
        m_Name = fs::path(m_jsonPath).stem().string();
    }
}

void CScene::BuildFallbackSceneIfMissing()
{
    if (m_Name == "SceneTitle" || id == Scenes::ID::TITLE)
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

        CObject* startBtn = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "StartButton");
        if (startBtn)
        {
            if (auto sprite = startBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_GameStart.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto btnComp = startBtn->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ChangeScene_Test);
            }
        }

        CObject* exitBtn = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "ExitButton");
        if (exitBtn)
        {
            if (auto sprite = exitBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_Exit.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto btnComp = exitBtn->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ExitGame);
            }
        }

        if (startBtn && exitBtn)
        {
            ButtonComponent* b1 = startBtn->GetComponent<ButtonComponent>();
            ButtonComponent* b2 = exitBtn->GetComponent<ButtonComponent>();
            if (b1 && b2)
            {
                b1->SetNavigationNames("ExitButton", "ExitButton", "", "");
                b2->SetNavigationNames("StartButton", "StartButton", "", "");
            }
        }

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);
        ButtonEventManager::GetInstance().SetFirstSelectedName("StartButton");
        SceneSerializer::SaveScene(m_jsonPath, id);
    }
    else if (m_Name == "SceneClear" || id == Scenes::ID::Clear || m_Name == "SceneFailed" || id == Scenes::ID::Failed)
    {
        bool isClear = (m_Name == "SceneClear" || id == Scenes::ID::Clear);
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera", "Camera");

        CObject* bgUI = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIImage", isClear ? "ClearBG" : "FaildBG");
        if (bgUI)
        {
            if (auto sprite = bgUI->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(isClear ? L"Assets/Texture/T_Clear.png" : L"Assets/Texture/T_Failed.png");
                sprite->SetSize(1920.0f, 1080.0f);
            }
        }

        CObject* retryBtn = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "RetryButton");
        if (retryBtn)
        {
            if (auto sprite = retryBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_Retry.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto btnComp = retryBtn->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ChangeScene_Test);
            }
        }

        CObject* toTitleBtn = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::UI, "UIButton", "ToTitleButton");
        if (toTitleBtn)
        {
            if (auto sprite = toTitleBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetTexture(L"Assets/Texture/T_ToTitle.png");
                sprite->SetSize(400.0f, 100.0f);
            }
            if (auto btnComp = toTitleBtn->AddComponent<ButtonComponent>())
            {
                btnComp->SetAction(ButtonAction::ChangeScene_Title);
            }
        }

        if (retryBtn && toTitleBtn)
        {
            ButtonComponent* b1 = retryBtn->GetComponent<ButtonComponent>();
            ButtonComponent* b2 = toTitleBtn->GetComponent<ButtonComponent>();
            if (b1 && b2)
            {
                b1->SetNavigationNames("ToTitleButton", "ToTitleButton", "", "");
                b2->SetNavigationNames("RetryButton", "RetryButton", "", "");
            }
        }

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);
        ButtonEventManager::GetInstance().SetFirstSelectedName("RetryButton");
        SceneSerializer::SaveScene(m_jsonPath, id);
    }
    else
    {
        // Default 3D Test Scene fallback
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");

        CObject* player = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::PLAYER, "Player");
        if (player)
        {
            if (auto t = player->GetComponent<CTransform>())
                t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
        }

        CObject* enemy = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::ENEMY, "Enemy");
        if (enemy)
        {
            if (auto t = enemy->GetComponent<CTransform>())
                t->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });
        }

        CObject* skydome = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BACKGROUND, "Skydome");
        if (skydome)
        {
            if (auto t = skydome->GetComponent<CTransform>())
                t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });
        }

        CObject* field = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::FIELD, "Field");
        if (field)
        {
            if (auto t = field->GetComponent<CTransform>())
                t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
        }

        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::MANAGER, "EnemyCounter");
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::LIGHT, "DirectionalLight", "DirectionalLight");

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);
        if (!m_jsonPath.empty())
        {
            SceneSerializer::SaveScene(m_jsonPath, id);
        }
    }
}

void CScene::ApplySceneUIEntranceEffects()
{
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

    if (m_Name == "SceneTitle" || id == Scenes::ID::TITLE)
    {
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

        CObject* startBtn = FindUIObject("StartButton");
        if (startBtn)
        {
            CRectTransform* rect = startBtn->GetComponent<CRectTransform>();
            if (!rect) rect = startBtn->AddComponent<CRectTransform>();
            rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
            rect->SetSizeDelta(400.0f, 100.0f);
            rect->SetAnchoredPosition(-20.0f, 260.0f);
            rect->SetScale(0.0f, 0.0f);

            rect->DOAnchorPos({ -20.0f, 150.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.2f);
            rect->DOScale({ 1.0f, 1.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.2f);

            if (auto sprite = startBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
                sprite->DOFade(1.0f, 0.4f)->SetDelay(0.2f);
            }

            if (auto btn = startBtn->GetComponent<ButtonComponent>())
            {
                ButtonEventManager::GetInstance().SetSelectedGameObject(btn);
            }
        }

        CObject* exitBtn = FindUIObject("ExitButton");
        if (exitBtn)
        {
            CRectTransform* rect = exitBtn->GetComponent<CRectTransform>();
            if (!rect) rect = exitBtn->AddComponent<CRectTransform>();
            rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
            rect->SetSizeDelta(400.0f, 100.0f);
            rect->SetAnchoredPosition(-20.0f, 390.0f);
            rect->SetScale(0.0f, 0.0f);

            rect->DOAnchorPos({ -20.0f, 290.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.35f);
            rect->DOScale({ 1.0f, 1.0f }, 0.6f)->SetEase(Ease::OutBack)->SetDelay(0.35f);

            if (auto sprite = exitBtn->GetComponent<CSpriteRenderer>())
            {
                sprite->SetColor({ 1.0f, 1.0f, 1.0f, 0.0f });
                sprite->DOFade(0.4f, 0.4f)->SetDelay(0.35f);
            }
        }
    }
    else if (m_Name == "SceneClear" || id == Scenes::ID::Clear || m_Name == "SceneFailed" || id == Scenes::ID::Failed)
    {
        CObject* bgObj = FindUIObject("ClearBG");
        if (!bgObj) bgObj = FindUIObject("FaildBG");
        if (!bgObj) bgObj = FindUIObject("FailedBG");
        if (bgObj)
        {
            CRectTransform* rect = bgObj->GetComponent<CRectTransform>();
            if (!rect) rect = bgObj->AddComponent<CRectTransform>();
            rect->SetAnchorPreset(AnchorPreset::StretchAll, true);
            rect->SetSizeDelta(0.0f, 0.0f);
            rect->SetScale(1.0f, 1.0f);
        }

        CObject* retryBtn = FindUIObject("RetryButton");
        if (retryBtn)
        {
            CRectTransform* rect = retryBtn->GetComponent<CRectTransform>();
            if (!rect) rect = retryBtn->AddComponent<CRectTransform>();
            rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
            rect->SetSizeDelta(400.0f, 100.0f);
            rect->SetAnchoredPosition(0.0f, 150.0f);
            rect->SetScale(1.0f, 1.0f);

            if (auto btn = retryBtn->GetComponent<ButtonComponent>())
            {
                ButtonEventManager::GetInstance().SetSelectedGameObject(btn);
            }
        }

        CObject* toTitleBtn = FindUIObject("ToTitleButton");
        if (toTitleBtn)
        {
            CRectTransform* rect = toTitleBtn->GetComponent<CRectTransform>();
            if (!rect) rect = toTitleBtn->AddComponent<CRectTransform>();
            rect->SetAnchorPreset(AnchorPreset::MiddleCenter, true);
            rect->SetSizeDelta(400.0f, 100.0f);
            rect->SetAnchoredPosition(0.0f, 290.0f);
            rect->SetScale(1.0f, 1.0f);
        }
    }
}

void CScene::Init()
{
    ButtonEventManager::GetInstance();

    if (m_jsonPath.empty())
    {
        m_jsonPath = DefaultPathForLegacyID(id);
        if (!m_jsonPath.empty())
        {
            m_Name = fs::path(m_jsonPath).stem().string();
        }
    }

    if (m_jsonPath.empty() || !SceneSerializer::LoadSceneOrDefault(m_jsonPath, id))
    {
        BuildFallbackSceneIfMissing();
    }

    ApplySceneUIEntranceEffects();

    // Detect if this scene is a pure 2D UI scene (no 3D Models, Fields, or Lights)
    bool has3DContent = false;
    const auto& allObjects = ObjectManager::GetInstance().GetObjectList();
    for (const auto& tagVec : allObjects)
    {
        for (const auto& obj : tagVec)
        {
            if (!obj || obj->GetIsDestroyed()) continue;
            if (obj->GetComponent<CModel>() || obj->GetComponent<FieldComponent>() || obj->GetComponent<LightComponent>())
            {
                has3DContent = true;
                break;
            }
        }
        if (has3DContent) break;
    }

    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice();
    const float sceneClearColor[4] = { 0.1f, 0.2f, 0.4f, 1.0f };
    m_pSceneTexture = std::make_unique<RenderTexture>(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM, sceneClearColor);

    m_renderPipeline = std::make_unique<RenderPipeline>();

    bool isDeferredScene = (m_Name.find("Deferred") != std::string::npos || m_jsonPath.find("Deferred") != std::string::npos);
    bool isPureUIScene = (!has3DContent) && (id == Scenes::ID::TITLE || id == Scenes::ID::Clear || id == Scenes::ID::Failed || m_Name == "SceneTitle" || m_Name == "SceneClear" || m_Name == "SceneFailed");

    if (isDeferredScene)
    {
        const std::string deferredPipelineJsonPath = "Assets/Pipeline/DeferredPipeline.json";
        if (!fs::exists(deferredPipelineJsonPath) ||
            !m_renderPipeline->LoadFromJson(deferredPipelineJsonPath, pDevice, m_pSceneTexture.get()))
        {
            m_renderPipeline->SetupDeferredPipeline(pDevice, m_pSceneTexture.get());
            m_renderPipeline->SaveToJson(deferredPipelineJsonPath);
        }
    }
    else if (isPureUIScene)
    {
        m_renderPipeline->GetBlackboard().RegisterTextureRef("SceneColor", m_pSceneTexture.get());
        m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(m_pSceneTexture.get()));
        m_renderPipeline->AddPass(std::make_unique<PostProcessPass>(m_pSceneTexture.get()));
        m_renderPipeline->Init(pDevice);
    }
    else
    {
        const std::string pipelineJsonPath = "Assets/Pipeline/GameScenePipeline.json";
        if (!fs::exists(pipelineJsonPath) ||
            !m_renderPipeline->LoadFromJson(pipelineJsonPath, pDevice, m_pSceneTexture.get()))
        {
            m_renderPipeline->SetupCommercialMultiLayerPipeline(pDevice, m_pSceneTexture.get());
            m_renderPipeline->SaveToJson(pipelineJsonPath);
        }
    }
}

void CScene::Update()
{
    ButtonEventManager::GetInstance().Update();

    ObjectManager::GetInstance().Update(Scenes::ID::NONE);
}

void CScene::Draw()
{
    if (m_renderPipeline)
    {
        RenderContext ctx;
        ctx.cmdList       = DX12Manager::GetInstance().GetCommandList();
        ctx.sceneID       = Scenes::ID::NONE;
        ctx.deltaTime     = TimeManager::GetInstance().GetDeltaTime();
        ctx.backBufferRTV = DX12Manager::GetInstance().GetCurrentBackBufferRTV();
        ctx.mainDSV       = DX12Manager::GetInstance().GetMainDSV();
        ctx.screenWidth   = SCREEN_WIDTH;
        ctx.screenHeight  = SCREEN_HEIGHT;

        m_renderPipeline->Execute(ctx);
    }
    else
    {
        ObjectManager::GetInstance().Draw(Scenes::ID::NONE);
    }
}
