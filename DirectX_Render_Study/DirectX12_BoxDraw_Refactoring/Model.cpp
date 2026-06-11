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

		//繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ縺後♀縺九＠縺九▲縺溘ｉ縲√％縺薙ｒ縺・  繧・
		//m_SkinningMatrices[i] =
		//	 m_Bones[i]->inverseBindPose* m_Bones[i]->globalPose;
	
}

void CModel::CreateTmpBoneData()
{
	//----- 繝・E繝ｳ縺ｮ繝・ E繧ｿ莉ｮ菴・E -----
	Bone bone;
	bone = std::make_shared<CBone>();

	bone->name = "Root";
	bone->parentIndex = -1;
	bone->children = {};

	// 蛻晄悄蟋ｿ蜍｢ E EindPose E E
	bone->localBindPose = DirectX::XMMatrixIdentity();

	// 騾・   E
	bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->localBindPose);

	// 迴ｾ蝨ｨ繝・E繧ｺ
	bone->localPose = DirectX::XMMatrixIdentity();
	bone->globalPose = DirectX::XMMatrixIdentity();
	m_Bones.push_back(bone);

	//繝・E繝ｳ繝舌ャ繝輔ぃ縺ｮ菴・E
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
	// 竭� 繝ｪ繧ｽ繝ｼ繧ｹ菴・E E EPLOAD E E
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
	// 竭｡ SRV縺ｮ逋ｻ骭ｲ
	//=============================

	m_boneSrvIndex = DX12Manager::GetInstance().AllocsrvNextIndex();

	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle =
		DX12Manager::GetInstance().GetCpuSrvHandle(m_boneSrvIndex);

	//========================
	// 竭｢ SRV菴・E
	//========================
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer縺ｯUNORM荳崎ｦ・
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = boneCount;
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_BoneBuffer.Get(), &srvDesc, cpuHandle);

	//========================
	// 竭｣ GPU繝上Φ繝峨Ν菫・E
	//========================
	// GPU繝上Φ繝峨Ν E Eraw()縺ｧ菴ｿ縺・  E
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

	//繝ｩ繝�繝髢｢謨ｰ縺ｮ螳夂ｾｩ
	auto lambdaComputeBindPose = [&](auto& self, int nodeIdx, const DirectX::XMMATRIX& parentMatrix) -> void 
	{
		//Bone縺ｮ蜿門ｾ・
		auto& bone = m_Bones[nodeIdx];

		// 繧ｰ繝ｭ繝ｼ繝舌Ν陦・E = 閾ｪ霄ｫ縺ｮ繝ｭ繝ｼ繧ｫ繝ｫ * 隕ｪ縺ｮ繧ｰ繝ｭ繝ｼ繝舌Ν
		bone->globalBindPose = bone->localBindPose * parentMatrix;
		bone->globalPose = bone->globalBindPose; // 迴ｾ蝨ｨ縺ｮ繝・E繧ｺ繧ょ酔?E

		// 蟄舌ヮ繝ｼ繝峨∈莨晄眺
		for (int childIdx : bone->children) 
		{
			self(self, childIdx, bone->globalBindPose);
		}
	};

	//隕ｪ縺後＞縺ｪ縺・  繝ｼ繝会ｼ・arentIndex == -1 縺ｮ Root繝・E繝会ｼ峨ｒ襍ｷ轤ｹ縺ｫ襍ｰ繧峨○繧・
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//-1縺瑚ｵｷ轤ｹ
		if (m_Bones[i]->parentIndex == -1)
		{
			lambdaComputeBindPose(lambdaComputeBindPose, i, DirectX::XMMatrixIdentity());
		}
	}

	//-- 4.SkinData 縺九ｉ豁｣蠑上↑ inverseBindPose E 騾・   E E 繧貞牡繧雁ｽ薙※繧・
	// 縺ｾ縺・E繝・E繝峨↓蟇ｾ縺励※縲√せ繝・  繝・縺ｮglobalBindPose縺ｮ騾・   E繧貞ｮ・E逕ｨ縺ｫ蜈･繧後※縺翫￥
	for (auto& bone : m_Bones)
	{
		bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->globalBindPose);
	}

	// 繧ｹ繧ｭ繝ｳ繝・ E繧ｿ E  E繝ｼ繝ｳ?E   E 縺後≠繧句�ｴ蜷医“LTF縺ｮ豁｣遒ｺ縺ｪ騾・  繧､繝ｳ繝芽｡・E縺ｧ荳頑嶌縺・
	if (!loadedModelData.skins.empty())
	{
		//蜊倅ｸ繧ｹ繧ｭ繝ｳ繧呈Φ螳・
		//蜊倅ｸ€繧ｹ繧ｭ繝ｳ繧呈Φ螳・
		const auto& skin = loadedModelData.skins[0]; // 繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ逕ｨ縺ｮ蜊倅ｸ€skin
		m_SkinJoints = skin.joints;

		for (size_t i = 0; i < skin.joints.size(); ++i)
		{
			int nodeIdx = skin.joints[i]; // skin上のi番目のボーンが指す、全ノード(m_Bones)の中のインデックス

			// GLTFの行列は列優先(column-major)なので、XMLoadFloat4x4で読むと自動的に行優先(row-major)に変換される。
			// そのため、ここではTransposeをしてはいけない！
			m_Bones[nodeIdx]->inverseBindPose = DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices[i]);
		}
	}

	//繝・E繝ｳ繝舌ャ繝輔ぃ菴・E
	CreateBoneBuffer();


	//----- 繝槭ユ繝ｪ繧｢繝ｫ菴・E -----
	//繝槭ユ繝ｪ繧｢繝ｫ莉ｮ菴・E
	UINT mat_num = RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f,1.0f,1.0f,1.0f });

	// -- 5.縺吶∋縺ｦ縺ｮ繝｡繝・  繝･縺ｮ逋ｻ骭ｲ E Ereak繧貞炎髯､縺励※蜈ｨ驛ｨ隱ｭ縺ｿ霎ｼ繧 E E  E
	for (int i = 0; i < loadedModelData.nodes.size(); ++i)
	{
		const auto& node = loadedModelData.nodes[i];

		if (node.meshIndex != -1)
		{
			const auto& mesh = loadedModelData.meshes[node.meshIndex];

			// 縺吶∋縺ｦ縺ｮ繝｡繝・  繝･ E E5?69逡ｪ縺ｪ縺ｩ E 繧坦egisterMesh縺励∪縺・
			RegisterMesh(
				mat_num,
				mesh.vertices.data(),
				mesh.vertices.size(),
				mesh.indices.data(),
				mesh.indices.size()
			);
		}
	}


	////Mesh縺ｮ逋ｻ骭ｲ
	////鬆らせ繝・ E繧ｿ繧よｸ｡縺帙ｋ繧医≧縺ｫ縺吶ｋ
	////蠑墓焚縺ｪ縺励〒莉ｮ繝・ E繧ｿ(Cube繧堤匳骭ｲ縺吶ｋ繧医≧縺ｫ縺吶ｋ)
	////繝｡繝・  繝･縺斐→縺ｫ繝ｫ繝ｼ繝励☆繧・
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



	//霑斐▲縺ｦ譚･縺溘ｂ縺ｮ縺九ｉ縲｀esh,Material,Bone縺ｮ繝・ E繧ｿ繧剃ｽ・E縺吶ｋ
	//MakeBones(繝・E繝・E繝・ E繧ｿ縺九ｉ菴・E) 
	//Skin縺ｮ?E  繧呈爾邏｢縺励※縲。one縺ｮ繧､繝ｳ繝・  繧ｯ繧ｹ縺ｮ繝ｪ繧ｹ繝医ｒ菴・E縺吶ｋ

	

	//MakeMashes(繝槭ユ繝ｪ繧｢繝ｫ縺ｮ繝・ E繧ｿ縺九ｉ菴・E)

	//MakeMaterials



}

void CModel::Init()
{

}

void CModel::Update() 
{
	//Mesh縺ｮ譖ｴ譁ｰ
	for (auto& mesh : m_Meshes)
	{
		mesh->Update();
	}
}

void CModel::Draw() 
{
	//繝・E繝ｳ縺ｮ譖ｴ譁ｰ
	UpdateBones();

	//static float time = 0.01f; // 驕ｩ蠖薙↓譎る俣
	//time += 0.01f;
	//DirectX::XMMATRIX rot = DirectX::XMMatrixRotationX(time);
	//m_Bones[0]->localPose = rot;

	// GPU縺ｸ繝・E繝ｳ陦・E繧帝√ｋ
	UpdateBoneBuffer();

	ID3D12GraphicsCommandList* commandList =
		DX12Manager::GetInstance().GetCommandList();

	
	ID3D12DescriptorHeap* heaps[] =
	{
		DX12Manager::GetInstance().GetSRVHeap()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	CTransform* transform = m_Owner->GetComponent<CTransform>();

	//Mesh縺ｮ謠冗判
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

//繝輔ぃ繧､繝ｫ繝・ E繧ｿ騾壹ｊ縺ｫ隱ｭ縺ｿ霎ｼ繧縺薙→蜑肴署
//濶ｲ縺�縺大､峨∴縺溘く繝｣繝ｩ繧ｯ繧ｿ繝ｼ繧堤畑諢上＠縺溘＞縺ｪ繧峨∽ｽ輔°謇区ｮｵ繧定・  繧・E  縺後≠繧九°繧・
UINT CModel::RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color)
{
	//Material縺ｮVector縺ｫ霑ｽ蜉�
	//縺薙％縺ｧ繝・  繧ｹ繝√Ε縺ｮ隱ｭ縺ｿ霎ｼ縺ｿ繧り｡後≧
	m_Materials.push_back(std::make_shared<CMaterial>(_FilePath, _Color));

	//LastIndex繧定ｿ斐○縺ｰ縺・  縺・繧ｭ繝｣繝・  繝･縺後≠繧・E隧ｱ縺ｯ蛻･縺九ｂ)
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
