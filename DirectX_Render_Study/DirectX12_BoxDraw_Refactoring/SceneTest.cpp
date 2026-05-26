#include "SceneTest.h"
#include "3D_Object.h"

#include "Object.h"

#include "Model.h"

CSceneTest::CSceneTest()
{
    ////--- Box ---
    //auto box = std::make_unique<C3D_Object>("Box");


    //CModel* model = box->GetComponent<CModel>();

    //model->CreateTmpBoneData();

    ////Transformの登録
    //box->SetTransform({ 0.0f, 2.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 0.0f });
    //
    ////Modelの取得

    ////Materialの登録
    //UINT mat_num = model->RegisterMatarial(L"Assets/Texture/Sample2.jpg", {1.0f,1.0f,1.0f,1.0f});
    //
    ////Meshの登録
    //model->RegisterMesh(mat_num);

    //m_VecObject.push_back(std::move(box));


    ////--- Ground ---
    //auto ground = std::make_unique<C3D_Object>("Ground");

    //model = ground->GetComponent<CModel>();

    //model->CreateTmpBoneData();

    //ground->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, 0.0f });


    ////Modelの取得

    ////Materialの登録
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////Meshの登録
    //model->RegisterMesh(mat_num);

    ////配列に追加
    //m_VecObject.push_back(std::move(ground));

    ////--- Slope ---
    //auto slope = std::make_unique<C3D_Object>("Slope");

    //model = slope->GetComponent<CModel>();


    //model->CreateTmpBoneData();

    //slope->SetTransform({ 0.0f, 0.0f, 10.0f }, { 20.0f, 0.1f, 20.0f }, { -0.8f, 0.0f, 0.0f });


    ////Modelの取得

    ////Materialの登録
    //mat_num = model->RegisterMatarial(L"Assets/Texture/Sample3.jpg", { 1.0f,1.0f,1.0f,1.0f });

    ////Meshの登録
    //model->RegisterMesh(mat_num);

    ////配列に追加
    //m_VecObject.push_back(std::move(slope));


    //----- Model -----
    auto model_obj = std::make_unique<C3D_Object>("Model_obj");

    CModel* model = model_obj->GetComponent<CModel>();

    model_obj->SetTransform({ 0.0f, 0.0f, 0.0f }, { 20.0f, 0.1f, 20.0f }, { 0.0f, 0.0f, -1.6f });

    model->ModelLoad("");


    //配列に追加
    m_VecObject.push_back(std::move(model_obj));

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
    //モデルによってDrawする
    for (auto& object : m_VecObject)
    {
        object->Draw();
    }
}

void CSceneTest::AddObject(UniquePtr<CObject> _Object)
{
    m_VecObject.push_back(std::move(_Object));
}
