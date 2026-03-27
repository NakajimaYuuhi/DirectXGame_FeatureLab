#include "Model.h"

CModel::CModel()
	:CComponent("Model")
{
}

void CModel::Init()
{

}

void CModel::Update() 
{
	m_Mesh->Update();
}

void CModel::Draw() 
{
	m_Mesh->Draw();
}

void CModel::RegisterMesh(UINT _MatIdx)
{


	m_Mesh = std::make_shared<CMesh>(m_Materials[_MatIdx].get());

	m_Mesh->RegisterOwner(m_Owner);
	m_Mesh->Init();
}

//ファイルデータ通りに読み込むこと前提
//色だけ変えたキャラクターを用意したいなら、何か手段を考える必要があるかも
UINT CModel::RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color)
{
	//MaterialのVectorに追加
	//ここでテクスチャの読み込みも行う
	m_Materials.push_back(std::make_shared<CMaterial>(_FilePath, _Color));

	//LastIndexを返せばいいか(キャッシュがあれば話は別かも)
	return m_Materials.size() - 1;
}
