#include "Model.h"
#include "DX12Manager.h"
#include "gltfLoader.h"

CModel::CModel()
	:CComponent("Model")
{


	

}

void CModel::CalculateRecursive(int index)
{
	auto& bone = m_Bones[index];

	if (bone->parentIndex < 0)
	{
		bone->globalPose = bone->localPose;
	}
	else
	{
		auto& parent = m_Bones[bone->parentIndex];
		bone->globalPose = parent->globalPose * bone->localPose;
	}

	for (int child : bone->children)
	{
		CalculateRecursive(child);
	}
}

void CModel::UpdateBones()
{
	// ① 親子伝播
	for (int i = 0; i < m_Bones.size(); i++)
	{
		if (m_Bones[i]->parentIndex < 0)
		{
			CalculateRecursive(i);
		}
	}

	// ② スキニング行列作成 ← ★ここ！！
	m_SkinningMatrices.resize(m_Bones.size());

	for (int i = 0; i < m_Bones.size(); i++)
	{
		m_SkinningMatrices[i] =
			m_Bones[i]->globalPose * m_Bones[i]->inverseBindPose;
	}
}

void CModel::CreateTmpBoneData()
{
	//----- ボーンのデータ仮作成 -----
	Bone bone;
	bone = std::make_shared<CBone>();

	bone->name = "Root";
	bone->parentIndex = -1;
	bone->children = {};

	// 初期姿勢（BindPose）
	bone->localBindPose = DirectX::XMMatrixIdentity();

	// 逆行列
	bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->localBindPose);

	// 現在ポーズ
	bone->localPose = DirectX::XMMatrixIdentity();
	bone->globalPose = DirectX::XMMatrixIdentity();
	m_Bones.push_back(bone);

	//ボーンバッファの作成
	CreateBoneBuffer();
}

void CModel::CreateBoneBuffer()
{
	ID3D12Device* device = CDX12Manager::GetInstance().GetDevice();
	ID3D12DescriptorHeap* srvHeap = CDX12Manager::GetInstance().GetSRVHeap();

	UINT boneCount = static_cast<UINT>(m_Bones.size());
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	//=============================
	// ① リソース作成（UPLOAD）
	//=============================
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);

	device->CreateCommittedResource(
		&heapProps,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_BoneBuffer)
	);

	//=============================
	// ② SRVの登録
	//=============================

	m_boneSrvIndex = CDX12Manager::GetInstance().AllocsrvNextIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle =
		CDX12Manager::GetInstance().GetCpuSrvHandle(m_boneSrvIndex);

	//========================
	// ③ SRV作成
	//========================
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBufferはUNORM不要
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = boneCount;
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_BoneBuffer.Get(), &srvDesc, cpuHandle);

	//========================
	// ④ GPUハンドル保存
	//========================
	// GPUハンドル（Draw()で使う）
	m_BoneSrvGpuHandle = CDX12Manager::GetInstance().GetGpuSrvHandle(m_boneSrvIndex);

}

void CModel::UpdateBoneBuffer()
{
	if (!m_BoneBuffer) return;

	UINT boneCount = (UINT)m_Bones.size();
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	// CPU側の m_SkinningMatrices が毎フレーム計算されている前提
	void* mapped = nullptr;

	m_BoneBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, m_SkinningMatrices.data(), bufferSize);
	m_BoneBuffer->Unmap(0, nullptr);
}

void CModel::ModelLoad(std::string _Path)
{
	//モデルのデータ
	LoadedModelData loadedModelData;
	
	//フォーマットに応じて、適切なLoaderを呼ぶ
	//一旦GLTFLoaderで実装する
	loadedModelData = TestLoadGLTF();

	//ボーンデータ仮作成
	CreateTmpBoneData();

	//マテリアル仮作成
	UINT mat_num = RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

	//Meshの登録
	//頂点データも渡せるようにする
	//引数なしで仮データ(Cubeを登録するようにする)
	//メッシュごとにループする
	int index = 0;
	RegisterMesh(mat_num, &(loadedModelData.meshes[0].vertices[0]), sizeof(loadedModelData.meshes[0].vertices) / sizeof(loadedModelData.meshes[0].vertices[0]),
		&(loadedModelData.meshes[0].indices[0]), sizeof(loadedModelData.meshes[0].indices) / sizeof(loadedModelData.meshes[0].indices[0]));
	//m_Meshes[0]->SetVertex(
	//	&(loadedModelData.meshes[0].vertices[0]), sizeof(loadedModelData.meshes[0].vertices) / sizeof(loadedModelData.meshes[0].vertices[0]),
	//	&(loadedModelData.meshes[0].indices[0]),  sizeof(loadedModelData.meshes[0].indices)  / sizeof(loadedModelData.meshes[0].indices[0])
	//	);



	//返って来たものから、Mesh,Material,Boneのデータを作成する
	//MakeBones(ノードのデータから作成) 
	//Skinの情報を探索して、Boneのインデックスのリストを作成する

	

	//MakeMashes(マテリアルのデータから作成)

	//MakeMaterials



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
	//ボーンの更新
	UpdateBones();

	static float time = 0.01f; // 適当に時間
	time += 0.01f;
	DirectX::XMMATRIX rot = DirectX::XMMatrixRotationY(time);
	m_Bones[0]->localPose = rot;

	// GPUへボーン行列を送る
	UpdateBoneBuffer();

	ID3D12GraphicsCommandList* commandList =
		CDX12Manager::GetInstance().GetCommandLIst();

	
	ID3D12DescriptorHeap* heaps[] =
	{
		CDX12Manager::GetInstance().GetSRVHeap()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	//Meshの描画
	for (auto& mesh : m_Meshes)
	{
		mesh->SetBoneSRV(m_BoneSrvGpuHandle);
		//mesh->BindBoneSRV(m_BoneSrvGpuHandle);
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

void CModel::RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount, const uint16_t* indices, size_t indexCount)
{
	//生成に失敗している
	Mesh mesh = std::make_shared<CMesh>(m_Materials[_MatIdx].get());

	mesh->RegisterOwner(m_Owner);
	mesh->SetVertex(
		vertices, vertexCount,
		indices, indexCount
	);
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
