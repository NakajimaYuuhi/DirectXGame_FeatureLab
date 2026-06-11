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
  		m_SkinningMatrices[i] = DirectX::XMMatrixTranspose(
  			m_Bones[i]->inverseBindPose * m_Bones[i]->globalPose);
	}

		//アニメーションがおかしかったら、ここをぁE  めE
		//m_SkinningMatrices[i] =
		//	 m_Bones[i]->inverseBindPose* m_Bones[i]->globalPose;
	
}

void CModel::CreateTmpBoneData()
{
	//----- チEEンのチE Eタ仮佁EE -----
	Bone bone;
	bone = std::make_shared<CBone>();

	bone->name = "Root";
	bone->parentIndex = -1;
	bone->children = {};

	// 初期姿勢 E EindPose E E
	bone->localBindPose = DirectX::XMMatrixIdentity();

	// 送E   E
	bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->localBindPose);

	// 現在チEEズ
	bone->localPose = DirectX::XMMatrixIdentity();
	bone->globalPose = DirectX::XMMatrixIdentity();
	m_Bones.push_back(bone);

	//チEEンバッファの佁EE
	CreateBoneBuffer();
}

void CModel::CreateBoneBuffer()
{
	ID3D12Device* device = DX12Manager::GetInstance().GetDevice();
	ID3D12DescriptorHeap* srvHeap = DX12Manager::GetInstance().GetSRVHeap();

	UINT boneCount = m_SkinJoints.empty() ? static_cast<UINT>(m_Bones.size()) : static_cast<UINT>(m_SkinJoints.size());
	if (boneCount == 0) boneCount = 1; // Prevent 0 size buffer
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	//=============================
	// �  リソース佁EE E EPLOAD E E
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
	// ③ SRV佁EE
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
	// ④ GPUハンドル俁EE
	//========================
	// GPUハンドル E Eraw()で使ぁE  E
	m_BoneSrvGpuHandle = DX12Manager::GetInstance().GetGpuSrvHandle(m_boneSrvIndex);

}

void CModel::UpdateBoneBuffer()
{
	if (!m_BoneBuffer) return;

	UINT boneCount = m_SkinJoints.empty() ? static_cast<UINT>(m_Bones.size()) : static_cast<UINT>(m_SkinJoints.size());
	if (boneCount == 0) boneCount = 1;
	UINT bufferSize = sizeof(DirectX::XMMATRIX) * boneCount;

	std::vector<DirectX::XMMATRIX> uploadMatrices(boneCount, DirectX::XMMatrixIdentity());
	if (m_SkinJoints.empty())
	{
		for (size_t i = 0; i < m_Bones.size(); i++)
		{
			uploadMatrices[i] = m_SkinningMatrices[i];
		}
	}
	else
	{
		for (size_t i = 0; i < m_SkinJoints.size(); ++i)
		{
			uploadMatrices[i] = m_SkinningMatrices[m_SkinJoints[i]];
		}
	}

	void* mapped = nullptr;

	m_BoneBuffer->Map(0, nullptr, &mapped);
	memcpy(mapped, uploadMatrices.data(), bufferSize);
	m_BoneBuffer->Unmap(0, nullptr);
}
void CModel::ModelLoad(std::string _Path)
{
	LoadedModelData loadedModelData;
	loadedModelData = TestLoadGLTF(_Path);

	m_Bones.clear();

	for (const auto& node : loadedModelData.nodes)
	{
		auto bone = std::make_shared<CBone>();
		bone->name = node.name;
		bone->children = node.children;
		bone->parentIndex = -1;
		
		bool hasMatrix = false;
		for (int i = 0; i < 16; ++i) { if (node.matrix[i] != 0.0f) { hasMatrix = true; break; } }

		if (hasMatrix)
		{
			bone->localBindPose = DirectX::XMLoadFloat4x4((const DirectX::XMFLOAT4X4*)node.matrix);
		}
		else
		{
			DirectX::XMVECTOR s = DirectX::XMVectorSet(node.scale[0], node.scale[1], node.scale[2], 0.0f);
			DirectX::XMVECTOR r = DirectX::XMVectorSet(node.rotation[0], node.rotation[1], node.rotation[2], node.rotation[3]);
			DirectX::XMVECTOR t = DirectX::XMVectorSet(node.translation[0], node.translation[1], node.translation[2], 0.0f);
			bone->localBindPose = DirectX::XMMatrixAffineTransformation(s, DirectX::XMVectorZero(), r, t);
		}

        DirectX::XMVECTOR s, r, t;
        DirectX::XMMatrixDecompose(&s, &r, &t, bone->localBindPose);
        DirectX::XMStoreFloat3(&bone->scale, s);
        DirectX::XMStoreFloat4(&bone->rotation, r);
        DirectX::XMStoreFloat3(&bone->translation, t);

		bone->localPose = bone->localBindPose;
		m_Bones.push_back(bone);
	}

	m_Animations = loadedModelData.animations;

	for (int i = 0; i < m_Bones.size(); ++i)
	{
		for (int childIdx : m_Bones[i]->children)
		{
			m_Bones[childIdx]->parentIndex = i;
		}
	}

	//ラ� ダ関数の定義
	auto lambdaComputeBindPose = [&](auto& self, int nodeIdx, const DirectX::XMMATRIX& parentMatrix) -> void 
	{
		//Boneの取征E
		auto& bone = m_Bones[nodeIdx];

		// グローバル衁EE = 自身のローカル * 親のグローバル
		bone->globalBindPose = bone->localBindPose * parentMatrix;
		bone->globalPose = bone->globalBindPose; // 現在のチEEズも同?E

		// 子ノードへ伝播
		for (int childIdx : bone->children) 
		{
			self(self, childIdx, bone->globalBindPose);
		}
	};

	//親がいなぁE  ード！EarentIndex == -1 の RootチEEド）を起点に走らせめE
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//-1が起点
		if (m_Bones[i]->parentIndex == -1)
		{
			lambdaComputeBindPose(lambdaComputeBindPose, i, DirectX::XMMatrixIdentity());
		}
	}

	//-- 4.SkinData から正式な inverseBindPose E 送E   E E を割り当てめE
	// まぁEEチEEドに対して、スチE  チEのglobalBindPoseの送E   Eを宁EE用に入れておく
	for (auto& bone : m_Bones)
	{
		bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->globalBindPose);
	}

	// スキンチE Eタ E  Eーン?E   E がある� �合、gLTFの正確な送E  インド衁EEで上書ぁE
	if (!loadedModelData.skins.empty())
	{
		//単一スキンを想宁E
		//単�?スキンを想宁E
		const auto& skin = loadedModelData.skins[0]; // キャラクター用の単�?skin
		m_SkinJoints = skin.joints;

		for (size_t i = 0; i < skin.joints.size(); ++i)
		{
			int nodeIdx = skin.joints[i]; // skin���i�Ԗڂ̃{�[�����w���A�S�m�[�h(m_Bones)�̒��̃C���f�b�N�X

			// GLTF�̍s��͗�D��(column-major)�Ȃ̂ŁAXMLoadFloat4x4�œǂނƎ����I�ɍs�D��(row-major)�ɕϊ������B
			// ���̂��߁A�����ł�Transpose�����Ă͂����Ȃ��I
			m_Bones[nodeIdx]->inverseBindPose = DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices[i]);
		}
	}

	//チEEンバッファ佁EE
	CreateBoneBuffer();


	//----- マテリアル佁EE -----
	//マテリアル仮佁EE
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



	//返って来たものから、Mesh,Material,BoneのチE Eタを佁EEする
	//MakeBones(チEEチEEチE Eタから佁EE) 
	//Skinの?E  を探索して、BoneのインチE  クスのリストを佁EEする

	

	//MakeMashes(マテリアルのチE Eタから佁EE)

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
	//チEEンの更新
	UpdateBones();

	//static float time = 0.01f; // 適当に時間
	//time += 0.01f;
	//DirectX::XMMATRIX rot = DirectX::XMMatrixRotationX(time);
	//m_Bones[0]->localPose = rot;

	// GPUへチEEン衁EEを送る
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
//色� け変えたキャラクターを用意したいなら、何か手段を老E  めEE  があるかめE
UINT CModel::RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color)
{
	//MaterialのVectorに追� 
	//ここでチE  スチャの読み込みも行う
	m_Materials.push_back(std::make_shared<CMaterial>(_FilePath, _Color));

	//LastIndexを返せばぁE  ぁEキャチE  ュがあめEE話は別かも)
	return m_Materials.size() - 1;
}

void CModel::UpdateAnimation(float deltaTime)
{
    if (m_currentAnimationIndex < 0 || m_currentAnimationIndex >= m_Animations.size() || m_Bones.empty()) return;

    m_animationTime += deltaTime;
    const AnimationData& anim = m_Animations[m_currentAnimationIndex];

    // Evaluate all channels
    for (const auto& channel : anim.channels) {
        if (channel.targetNodeIndex < 0 || channel.targetNodeIndex >= m_Bones.size()) continue;

        const AnimationSamplerData& sampler = anim.samplers[channel.samplerIndex];
        if (sampler.input.empty()) continue;

        // Loop animation
        float maxTime = sampler.input.back();
        float localTime = fmod(m_animationTime, maxTime);

        // Find keyframe
        size_t frameIdx = 0;
        for (size_t i = 0; i < sampler.input.size() - 1; ++i) {
            if (localTime >= sampler.input[i] && localTime < sampler.input[i + 1]) {
                frameIdx = i;
                break;
            }
        }

        size_t nextIdx = frameIdx + 1;
        if (nextIdx >= sampler.input.size()) nextIdx = frameIdx;

        float t0 = sampler.input[frameIdx];
        float t1 = sampler.input[nextIdx];
        float factor = (t1 > t0) ? (localTime - t0) / (t1 - t0) : 0.0f;

        const auto& v0 = sampler.output[frameIdx];
        const auto& v1 = sampler.output[nextIdx];

        auto& bone = m_Bones[channel.targetNodeIndex];

        if (channel.path == AnimationPath::TRANSLATION) {
            DirectX::XMVECTOR p0 = DirectX::XMVectorSet(v0[0], v0[1], v0[2], 0.0f);
            DirectX::XMVECTOR p1 = DirectX::XMVectorSet(v1[0], v1[1], v1[2], 0.0f);
            DirectX::XMVECTOR p = DirectX::XMVectorLerp(p0, p1, factor);
            DirectX::XMStoreFloat3(&bone->translation, p);
        }
        else if (channel.path == AnimationPath::ROTATION) {
            DirectX::XMVECTOR q0 = DirectX::XMVectorSet(v0[0], v0[1], v0[2], v0[3]);
            DirectX::XMVECTOR q1 = DirectX::XMVectorSet(v1[0], v1[1], v1[2], v1[3]);
            DirectX::XMVECTOR q = DirectX::XMQuaternionSlerp(q0, q1, factor);
            DirectX::XMStoreFloat4(&bone->rotation, q);
        }
        else if (channel.path == AnimationPath::SCALE) {
            DirectX::XMVECTOR s0 = DirectX::XMVectorSet(v0[0], v0[1], v0[2], 0.0f);
            DirectX::XMVECTOR s1 = DirectX::XMVectorSet(v1[0], v1[1], v1[2], 0.0f);
            DirectX::XMVECTOR s = DirectX::XMVectorLerp(s0, s1, factor);
            DirectX::XMStoreFloat3(&bone->scale, s);
        }
        
        bone->UpdateLocalPose();
    }

    UpdateBones();
}
