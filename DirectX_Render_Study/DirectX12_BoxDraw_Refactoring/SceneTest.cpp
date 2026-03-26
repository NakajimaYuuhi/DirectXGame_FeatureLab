#include "SceneTest.h"
#include "3D_Object.h"

#include "Object.h"

CSceneTest::CSceneTest()
{
    //Boxの作成
    auto box = std::make_unique<C3D_Object>("Box");

    box->SetTransform({ 0.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    
    m_VecObject.push_back(std::move(box));

    //Ground
    auto ground = std::make_unique<C3D_Object>("Ground");

    ground->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });

    m_VecObject.push_back(std::move(ground));

    //Slope
    auto slope = std::make_unique<C3D_Object>("Slope");

    slope->SetTransform({ 0.0f, 0.0f, 10.0f }, { 20.0f, 0.1f, 20.0f }, { -0.8f, 0.0f, 0.0f });

    m_VecObject.push_back(std::move(slope));
}



CSceneTest::~CSceneTest() = default;

void CSceneTest::Init()
{
    //それぞれのオブジェクトを初期化
    for (auto& object : m_VecObject)
    {
        object->Init();
    }
}

void CSceneTest::Update() 
{
    for (auto& object : m_VecObject)
    {
        object->Update();
    }
}

void CSceneTest::Draw() 
{
    for (auto& object : m_VecObject)
    {
        object->Draw();
    }
}

void CSceneTest::AddObject(UniquePtr<CObject> _Object)
{
    m_VecObject.push_back(std::move(_Object));
}
