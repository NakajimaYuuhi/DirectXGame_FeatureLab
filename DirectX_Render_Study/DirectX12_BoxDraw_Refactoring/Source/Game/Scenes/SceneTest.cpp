#include "SceneTest.h"
#include "Object.h"
#include "Model.h"
#include "DX12Manager.h"
#include "Transform.h"
#include "ObjectManager.h"
#include "InputManager.h"
#include "EventManager.h"
#include "EventData_NextScene.h"
#include "Source/Core/Scenes/Serializer/SceneSerializer.h"
#include "ShadowMapPass.h"
#include "ForwardRenderPass.h"
#include "PostProcessPass.h"
#include "TimeManager.h"
#include "BasicSettings.h"

CSceneTest::CSceneTest()
    :CScene(Scenes::ID::TEST)
{
}

CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{
    if (!SceneSerializer::LoadSceneOrDefault("Assets/Scene/SceneTest.json", Scenes::ID::TEST))
    {
        ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::CAMERA, "Camera");

        CObject* player = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::PLAYER, "Player");
        if (player) {
            CTransform* t = player->GetComponent<CTransform>();
            if (t) t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
        }

        CObject* enemy = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::ENEMY, "Enemy");
        if (enemy) {
            CTransform* t = enemy->GetComponent<CTransform>();
            if (t) t->SetTransform({ 0.0f, 0.0f, 10.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });
        }

        CObject* skydome = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::BACKGROUND, "Skydome");
        if (skydome) {
            if (auto t = skydome->GetComponent<CTransform>()) t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 3.14f, 0.0f });
        }

        CObject* field = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::FIELD, "Field");
        if (field) {
            if (auto t = field->GetComponent<CTransform>()) t->SetTransform({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
        }

        CObject* enemyCounter = ObjectManager::GetInstance().Instantiate(Scenes::ID::NONE, ObjectTag::MANAGER, "EnemyCounter");

        ObjectManager::GetInstance().Init(Scenes::ID::NONE);

        SceneSerializer::SaveScene("Assets/Scene/SceneTest.json", Scenes::ID::TEST);
    }

    ID3D12Device* pDevice = DX12Manager::GetInstance().GetDevice();
    m_pSceneTexture = std::make_unique<RenderTexture>(pDevice, SCREEN_WIDTH, SCREEN_HEIGHT, DXGI_FORMAT_R8G8B8A8_UNORM);

    m_renderPipeline = std::make_unique<RenderPipeline>();
    m_renderPipeline->AddPass(std::make_unique<ShadowMapPass>());
    m_renderPipeline->AddPass(std::make_unique<ForwardRenderPass>(m_pSceneTexture.get()));
    m_renderPipeline->AddPass(std::make_unique<PostProcessPass>(m_pSceneTexture.get()));
    m_renderPipeline->Init(pDevice);
}

void CSceneTest::Update() 
{
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

void CSceneTest::Draw() 
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
