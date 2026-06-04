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

		//アニメーションがおかしかったら、ここをいじる
		//m_SkinningMatrices[i] =
		//	 m_Bones[i]->inverseBindPose* m_Bones[i]->globalPose;
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
	loadedModelData = TestLoadGLTF(_Path);

	////ボーンデータ仮作成
	//CreateTmpBoneData();

	//----- ボーンデータ作成 -----
	//NodeDataからCBoneのVectorに一括取り込み

	//一旦ボーンをクリア
	m_Bones.clear();

	// -- 1.forループで全データ取り込み
	for (const auto& node : loadedModelData.nodes)
	{
		//領域作成
		auto bone = std::make_shared<CBone>();

		//Name,Childrenを一旦入れる
		bone->name = node.name;
		bone->children = node.children;
		bone->parentIndex = -1; // ステップ2で埋めるので一旦-1

		// --- ローカル初期行列（localBindPose）の作成 ---
		
		//行列で持っているかを確認
		bool hasMatrix = false;
		//1つでも値が入っているなら、持っているということ
		for (int i = 0; i < 16; ++i) { if (node.matrix[i] != 0.0f) { hasMatrix = true; break; } }

		if (hasMatrix)
		{
			bone->localBindPose = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)node.matrix);
		}
		else
		{
			// TRS（初期値）からローカル行列を合成
			DirectX::XMVECTOR s = DirectX::XMVectorSet(node.scale[0], node.scale[1], node.scale[2], 0.0f);
			DirectX::XMVECTOR r = DirectX::XMVectorSet(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]); // クォータニオン
			DirectX::XMVECTOR t = DirectX::XMVectorSet(node.translation[0], node.translation[1], node.translation[2], 0.0f);

			//行列に変換
			bone->localBindPose = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), r, t);
		}

		// 最初は現在のアニメーション用ローカルポーズ(localPose)も初期姿勢と同じにする
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

	//-- 3.初期グローバル行列(globalBindPose)の階層計算

	//ラムダ関数を使ってRootから再帰的に計算
	//ラムダ関数の定義
	auto lambdaComputeBindPose = [&](auto& self, int nodeIdx, const DirectX::XMMATRIX& parentMatrix) -> void 
	{
		//Boneの取得
		auto& bone = m_Bones[nodeIdx];

		// グローバル行列 = 自身のローカル * 親のグローバル
		bone->globalBindPose = bone->localBindPose * parentMatrix;
		bone->globalPose = bone->globalBindPose; // 現在のポーズも同期

		// 子ノードへ伝播
		for (int childIdx : bone->children) 
		{
			self(self, childIdx, bone->globalBindPose);
		}
	};

	//親がいないノード（parentIndex == -1 の Rootノード）を起点に走らせる
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//-1が起点
		if (m_Bones[i]->parentIndex == -1)
		{
			lambdaComputeBindPose(lambdaComputeBindPose, i, DirectX::XMMatrixIdentity());
		}
	}

	//-- 4.SkinData から正式な inverseBindPose（逆行列）を割り当てる
	// まず全ノードに対して、ステップ3のglobalBindPoseの逆行列を安全用に入れておく
	for (auto& bone : m_Bones)
	{
		bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->globalBindPose);
	}

	// スキンデータ（ボーン情報）がある場合、gLTFの正確な逆バインド行列で上書き
	if (!loadedModelData.skins.empty())
	{
		//ここは定数
		const auto& skin = loadedModelData.skins[0]; // キャラクター用のskin

		for (size_t i = 0; i < skin.joints.size(); ++i)
		{
			int nodeIdx = skin.joints[i]; // skin上のi番目のボーンが指す、全ノード(m_Bones)の中のインデックス

			// tinygltfから読み込んだ XMFLOAT4X4 を XMMATRIX に変換して上書き
			m_Bones[nodeIdx]->inverseBindPose = DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices[i]);
		}
	}

	//ボーンバッファ作成
	CreateBoneBuffer();


	//----- マテリアル作成 -----
	//マテリアル仮作成
	UINT mat_num = RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

	// -- 5.すべてのメッシュの登録（breakを削除して全部読み込む！）
	for (int i = 0; i < loadedModelData.nodes.size(); ++i)
	{
		const auto& node = loadedModelData.nodes[i];

		if (node.meshIndex != -1)
		{
			const auto& mesh = loadedModelData.meshes[node.meshIndex];

			// すべてのメッシュ（65?69番など）をRegisterMeshします
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
	////頂点データも渡せるようにする
	////引数なしで仮データ(Cubeを登録するようにする)
	////メッシュごとにループする
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

	//static float time = 0.01f; // 適当に時間
	//time += 0.01f;
	//DirectX::XMMATRIX rot = DirectX::XMMatrixRotationX(time);
	//m_Bones[0]->localPose = rot;

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

void CModel::RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount, const uint32_t* indices, size_t indexCount)
{
	//生成に失敗している
	Mesh mesh = std::make_shared<CMesh>(m_Materials[_MatIdx].get());

	mesh->RegisterOwner(m_Owner);
	mesh->Init();
	mesh->SetVertex(
		vertices, vertexCount,
		indices, indexCount
	);

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
