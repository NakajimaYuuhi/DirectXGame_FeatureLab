#include "SceneTest.h"
#include "3D_Object.h"

#include "Object.h"

#include "Model.h"

CSceneTest::CSceneTest()
{
    //--- Box ---
    auto box = std::make_unique<C3D_Object>("Box");

    //Transformの登録
    box->SetTransform({ 0.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    
    //Modelの取得
    CModel* model = box->GetComponent<CModel>();

    //Materialの登録
    UINT mat_num = model->RegisterMatarial(L"Assets/Texture/Sample2.jpg", {1.0f,1.0f,1.0f,1.0f});
    
    //Meshの登録
    model->RegisterMesh(mat_num);

    m_VecObject.push_back(std::move(box));


    //--- Ground ---
    auto ground = std::make_unique<C3D_Object>("Ground");

    ground->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });


    //Modelの取得
    model = ground->GetComponent<CModel>();

    //Materialの登録
    mat_num = model->RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

    //Meshの登録
    model->RegisterMesh(mat_num);

    //配列に追加
    m_VecObject.push_back(std::move(ground));

    //--- Slope ---
    auto slope = std::make_unique<C3D_Object>("Slope");

    slope->SetTransform({ 0.0f, 0.0f, 10.0f }, { 20.0f, 0.1f, 20.0f }, { -0.8f, 0.0f, 0.0f });


    //Modelの取得
    model = slope->GetComponent<CModel>();

    //Materialの登録
    mat_num = model->RegisterMatarial(L"Assets/Texture/Sample3.jpg", { 1.0f,1.0f,1.0f,1.0f });

    //Meshの登録
    model->RegisterMesh(mat_num);

    //配列に追加
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
