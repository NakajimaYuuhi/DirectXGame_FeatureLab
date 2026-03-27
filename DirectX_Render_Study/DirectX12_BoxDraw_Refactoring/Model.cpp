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
	//Meshの更新
	for (auto& mesh : m_Meshes)
	{
		mesh->Update();
	}
}

void CModel::Draw() 
{
	//Meshの描画
	for (auto& mesh : m_Meshes)
	{
		mesh->Draw();
	}
}

void CModel::RegisterMesh(UINT _MatIdx)
{
	Mesh mesh = std::make_shared<CMesh>(m_Materials[_MatIdx].get());

	mesh->RegisterOwner(m_Owner);
	mesh->Init();

	m_Meshes.push_back(mesh);
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
