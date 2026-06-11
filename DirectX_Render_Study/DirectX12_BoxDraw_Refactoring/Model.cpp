#include "Model.h"
#include "DX12Manager.h"
#include "gltfLoader.h"
#include "Transform.h"
#include "Object.h"

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
		bone->globalPose = bone->localPose * parent->globalPose;
	}

	for (int child : bone->children)
	{
		CalculateRecursive(child);
	}
}

void CModel::UpdateBones()
{
	for (int i = 0; i < m_Bones.size(); i++)
	{
		if (m_Bones[i]->parentIndex < 0)
		{
			CalculateRecursive(i);
		}
	}

	m_SkinningMatrices.resize(m_Bones.size());

	for (int i = 0; i < m_Bones.size(); i++)
	{
		m_SkinningMatrices[i] =
			m_Bones[i]->globalPose * m_Bones[i]->inverseBindPose;

		//アニメーションがおかしかったら、ここをぁE  めE
		//m_SkinningMatrices[i] =
		//	 m_Bones[i]->inverseBindPose* m_Bones[i]->globalPose;
	}
}

void CModel::CreateTmpBoneData()
{
	//----- ボ EンのチE Eタ仮作 E -----
	Bone bone;
	bone = std::make_shared<CBone>();

	bone->name = "Root";
	bone->parentIndex = -1;
	bone->children = {};

	// 初期姿勢 E EindPose E E
	bone->localBindPose = DirectX::XMMatrixIdentity();

	// 送E   E
	bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->localBindPose);

	// 現在ポ Eズ
	bone->localPose = DirectX::XMMatrixIdentity();
	bone->globalPose = DirectX::XMMatrixIdentity();
	m_Bones.push_back(bone);

	//ボ Eンバッファの作 E
	CreateBoneBuffer();
}

void CModel::CreateBoneBuffer()
{
	ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
	ID3D12DescriptorHeap* srvHeap = DX12Manager::GetInstance().GetSRVHeap();

	UINT boneCount = static_cast<UINT>(m_Bones.size());
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	//=============================
	// ① リソース作 E E EPLOAD E E
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

	m_boneSrvIndex = DX12Manager::GetInstance().AllocsrvNextIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle =
		DX12Manager::GetInstance().GetCpuSrvHandle(m_boneSrvIndex);

	//========================
	// ③ SRV作 E
	//========================
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBufferはUNORM不要E
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = boneCount;
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_BoneBuffer.Get(), &srvDesc, cpuHandle);

	//========================
	// ④ GPUハンドル保?E
	//========================
	// GPUハンドル E Eraw()で使ぁE  E
	m_BoneSrvGpuHandle = DX12Manager::GetInstance().GetGpuSrvHandle(m_boneSrvIndex);

}

void CModel::UpdateBoneBuffer()
{
	if (!m_BoneBuffer) return;

	UINT boneCount = (UINT)m_Bones.size();
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	// CPU側の m_SkinningMatrices が毎フレーム計算されてぁE  前提
	void* mapped = nullptr;

	m_BoneBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, m_SkinningMatrices.data(), bufferSize);
	m_BoneBuffer->Unmap(0, nullptr);
}

void CModel::ModelLoad(std::string _Path)
{
	//モチE  のチE Eタ
	LoadedModelData loadedModelData;
	
	//フォーマットに応じて、E  ?E  Loaderを呼ぶ
	//一旦GLTFLoaderで実裁E  めE
	loadedModelData = TestLoadGLTF(_Path);

	////ボ EンチE Eタ仮作 E
	//CreateTmpBoneData();

	//----- ボ EンチE Eタ作 E -----
	//NodeDataからCBoneのVectorに一括取り込み

	//一旦ボ Eンをクリア
	m_Bones.clear();

	// -- 1.forループで全チE Eタ取り込み
	for (const auto& node : loadedModelData.nodes)
	{
		//領域作 E
		auto bone = std::make_shared<CBone>();

		//Name,Childrenを一旦入れる
		bone->name = node.name;
		bone->children = node.children;
		bone->parentIndex = -1; // スチE  チEで埋める Eで一旦-1

		// --- ローカル初期行 E E EocalBindPose E  E作 E ---
		
		//行 Eで持ってぁE  かを確?E
		bool hasMatrix = false;
		//1つでも値が EってぁE  なら、持ってぁE  とぁE  こと
		for (int i = 0; i < 16; ++i) { if (node.matrix[i] != 0.0f) { hasMatrix = true; break; } }

		if (hasMatrix)
		{
			bone->localBindPose = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)node.matrix);
		}
		else
		{
			// TRS E  E期値 E からローカル行 Eを合?E
			DirectX::XMVECTOR s = DirectX::XMVectorSet(node.scale[0], node.scale[1], node.scale[2], 0.0f);
			DirectX::XMVECTOR r = DirectX::XMVectorSet(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]); // クォータニオン
			DirectX::XMVECTOR t = DirectX::XMVectorSet(node.translation[0], node.translation[1], node.translation[2], 0.0f);

			//行 Eに変換
			bone->localBindPose = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), r, t);
		}

		// 最初 E現在のアニメーション用ローカルポ Eズ(localPose)も E期姿勢と同じにする
		bone->localPose = bone->localBindPose;

		//プッシュする
		m_Bones.push_back(bone);
	}

	// -- 2.ParentDataを埋める
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//Childrenに該当するParentを書き換える
		for (int childIdx : m_Bones[i]->children)
		{
			m_Bones[childIdx]->parentIndex = i;
		}
	}

	//-- 3.初期グローバル行 E(globalBindPose)の階層計?E

	//ラムダ関数を使ってRootから再帰?E  計?E
	//ラムダ関数の定義
	auto lambdaComputeBindPose = [&](auto& self, int nodeIdx, const DirectX::XMMATRIX& parentMatrix) -> void 
	{
		//Boneの取征E
		auto& bone = m_Bones[nodeIdx];

		// グローバル行 E = 自身のローカル * 親のグローバル
		bone->globalBindPose = bone->localBindPose * parentMatrix;
		bone->globalPose = bone->globalBindPose; // 現在のポ Eズも同?E

		// 子ノードへ伝播
		for (int childIdx : bone->children) 
		{
			self(self, childIdx, bone->globalBindPose);
		}
	};

	//親がいなぁE  ード！EarentIndex == -1 の Rootノ Eド）を起点に走らせめE
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//-1が起点
		if (m_Bones[i]->parentIndex == -1)
		{
			lambdaComputeBindPose(lambdaComputeBindPose, i, DirectX::XMMatrixIdentity());
		}
	}

	//-- 4.SkinData から正式な inverseBindPose E 送E   E E を割り当てめE
	// まず Eノ Eドに対して、スチE  チEのglobalBindPoseの送E   Eを安 E用に入れておく
	for (auto& bone : m_Bones)
	{
		bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->globalBindPose);
	}

	// スキンチE Eタ E  Eーン?E   E がある場合、gLTFの正確な送E  インド行 Eで上書ぁE
	if (!loadedModelData.skins.empty())
	{
		//ここは定数
		const auto& skin = loadedModelData.skins[0]; // キャラクター用のskin

		for (size_t i = 0; i < skin.joints.size(); ++i)
		{
			int nodeIdx = skin.joints[i]; // skin上 Ei番目のボ Eンが指す、 Eノ EチEm_Bones)の中のインチE  クス

			// tinygltfから読み込んだ XMFLOAT4X4 めEXMMATRIX に変換して上書ぁE
			m_Bones[nodeIdx]->inverseBindPose = DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices[i]);
		}
	}

	//ボ Eンバッファ作 E
	CreateBoneBuffer();


	//----- マテリアル作 E -----
	//マテリアル仮作 E
	UINT mat_num = RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

	// -- 5.すべてのメチE  ュの登録 E Ereakを削除して全部読み込む E E  E
	for (int i = 0; i < loadedModelData.nodes.size(); ++i)
	{
		const auto& node = loadedModelData.nodes[i];

		if (node.meshIndex != -1)
		{
			const auto& mesh = loadedModelData.meshes[node.meshIndex];

			// すべてのメチE  ュ E E5?69番など E をRegisterMeshしまぁE
			RegisterMesh(
				mat_num,
				mesh.vertices.data(),
				mesh.vertices.size(),
				mesh.indices.data(),
				mesh.indices.size()
			);
		}
	}


	////Meshの登録
	////頂点チE Eタも渡せるようにする
	////引数なしで仮チE Eタ(Cubeを登録するようにする)
	////メチE  ュごとにループすめE
	//for (auto mesh : loadedModelData.meshes)
	//{


	//	RegisterMesh(
	//		mat_num, 
	//		mesh.vertices.data(), 
	//		mesh.vertices.size(),
	//		mesh.indices .data(), 
	//		mesh.indices .size()
	//	);
	//	break;
	//}




	//int index = 2;
	//RegisterMesh(mat_num, &(loadedModelData.meshes[index].vertices[0]), loadedModelData.meshes[index].vertices.size(),
	//	&(loadedModelData.meshes[index].indices[0]), loadedModelData.meshes[index].indices.size());
	//m_Meshes[0]->SetVertex(
	//	&(loadedModelData.meshes[index].vertices[0]), loadedModelData.meshes[0].vertices.size(),
	//	&(loadedModelData.meshes[index].indices[0]), loadedModelData.meshes[0].indices.size()
	//	);



	//返って来たものから、Mesh,Material,BoneのチE Eタを作 Eする
	//MakeBones(ノ Eド EチE Eタから作 E) 
	//Skinの?E  を探索して、BoneのインチE  クスのリストを作 Eする

	

	//MakeMashes(マテリアルのチE Eタから作 E)

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
	//ボ Eンの更新
	UpdateBones();

	//static float time = 0.01f; // 適当に時間
	//time += 0.01f;
	//DirectX::XMMATRIX rot = DirectX::XMMatrixRotationX(time);
	//m_Bones[0]->localPose = rot;

	// GPUへボ Eン行 Eを送る
	UpdateBoneBuffer();

	ID3D12GraphicsCommandList* commandList =
		DX12Manager::GetInstance().GetCommandList();

	
	ID3D12DescriptorHeap* heaps[] =
	{
		DX12Manager::GetInstance().GetSRVHeap()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	CTransform* transform = m_Owner->GetComponent<CTransform>();

	//Meshの描画
	for (size_t i = 0; i < m_Meshes.size(); ++i)
	{
		m_Meshes[i]->SetBoneSRV(m_BoneSrvGpuHandle);
		
		CMaterial* mat = nullptr;
		if (i < m_MeshMaterialIndices.size() && m_MeshMaterialIndices[i] < m_Materials.size())
		{
			mat = m_Materials[m_MeshMaterialIndices[i]].get();
		}

		m_Meshes[i]->Draw(transform, mat, m_BlendMode);
	}
}

void CModel::RegisterMesh(UINT _MatIdx)
{
	Mesh mesh = std::make_shared<CMesh>();
	
	mesh->RegisterOwner(m_Owner);
	mesh->Init();

	m_Meshes.push_back(mesh);
	m_MeshMaterialIndices.push_back(_MatIdx);
}

void CModel::RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
{
	Mesh mesh = std::make_shared<CMesh>();

	mesh->RegisterOwner(m_Owner);
	mesh->Init();
	mesh->SetVertex(vertices, vertexCount, indices, indexCount);

	m_Meshes.push_back(mesh);
	m_MeshMaterialIndices.push_back(_MatIdx);
}

//ファイルチE Eタ通りに読み込むこと前提
//色だけ変えたキャラクターを用意したいなら、何か手段を老E  る?E  があるかも
UINT CModel::RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color)
{
	//MaterialのVectorに追加
	//ここでチE  スチャの読み込みも行う
	m_Materials.push_back(std::make_shared<CMaterial>(_FilePath, _Color));

	//LastIndexを返せばぁE  ぁEキャチE  ュがあれ E話は別かも)
	return m_Materials.size() - 1;
}
