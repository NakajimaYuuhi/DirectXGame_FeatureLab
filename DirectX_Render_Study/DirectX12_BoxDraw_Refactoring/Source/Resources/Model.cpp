#include "Model.h"
#include "DX12Manager.h"
#include "gltfLoader.h"
#include "Transform.h"
#include "Object.h"
#include <algorithm>
#include <cmath>

CModel::CModel()
	:CComponent("Model")
{


	

}

CModel::~CModel()
{
	if (m_BoneSrvCpuHandle.ptr != 0 && m_BoneSrvGpuHandle.ptr != 0) {
		DX12Manager::GetInstance().GetSRVAllocator()->Free(m_BoneSrvCpuHandle, m_BoneSrvGpuHandle);
		m_BoneSrvCpuHandle.ptr = 0;
		m_BoneSrvGpuHandle.ptr = 0;
	}
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

		//?A?j???[?V????????????????????A???????E  ??E
		//m_SkinningMatrices[i] =
		//	 m_Bones[i]->inverseBindPose* m_Bones[i]->globalPose;
	
}

void CModel::CreateTmpBoneData()
{
	//----- ?`EE????`E E?^???EE -----
	Bone bone;
	bone = std::make_shared<CBone>();

	bone->name = "Root";
	bone->parentIndex = -1;
	bone->children = {};

	// ?????p?? E EindPose E E
	bone->localBindPose = DirectX::XMMatrixIdentity();

	// ??E   E
	bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->localBindPose);

	// ????`EE?Y
	bone->localPose = DirectX::XMMatrixIdentity();
	bone->globalPose = DirectX::XMMatrixIdentity();
	m_Bones.push_back(bone);

	//?`EE???o?b?t?@???EE
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
	//    ???\?[?X?EE E EPLOAD E E
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
	// ?A SRV??o?^
	//=============================

	DX12Manager::GetInstance().GetSRVAllocator()->Alloc(&m_BoneSrvCpuHandle, &m_BoneSrvGpuHandle);

	//========================
	// ?B SRV?EE
	//========================
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_UNKNOWN; // StructuredBuffer??UNORM?s?vE
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = boneCount;
	srvDesc.Buffer.StructureByteStride = sizeof(DirectX::XMMATRIX);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	device->CreateShaderResourceView(m_BoneBuffer.Get(), &srvDesc, m_BoneSrvCpuHandle);

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

	//??  ?_??????`
	auto lambdaComputeBindPose = [&](auto& self, int nodeIdx, const DirectX::XMMATRIX& parentMatrix) -> void 
	{
		//Bone???E
		auto& bone = m_Bones[nodeIdx];

		// ?O???[?o???EE = ???g????[?J?? * ?e??O???[?o??
		bone->globalBindPose = bone->localBindPose * parentMatrix;
		bone->globalPose = bone->globalBindPose; // ?????`EE?Y????E

		// ?q?m?[?h??`?d
		for (int childIdx : bone->children) 
		{
			self(self, childIdx, bone->globalBindPose);
		}
	};

	//?e???????E  ?[?h?IEarentIndex == -1 ?? Root?`EE?h?j??N?_????У_??E
	for (int i = 0; i < m_Bones.size(); ++i)
	{
		//-1???N?_
		if (m_Bones[i]->parentIndex == -1)
		{
			lambdaComputeBindPose(lambdaComputeBindPose, i, DirectX::XMMatrixIdentity());
		}
	}

	//-- 4.SkinData ???Ч‘???? inverseBindPose E ??E   E E ????дн???E
	// ???EE?`EE?h??????A?X?`E  ?`E??globalBindPose???E   E???EE?p?????????
	for (auto& bone : m_Bones)
	{
		bone->inverseBindPose = DirectX::XMMatrixInverse(nullptr, bone->globalBindPose);
	}

	// ?X?L???`E E?^ E  E?[???E   E ??????   ???AgLTF????m???E  ?C???h?EE?????E
	if (!loadedModelData.skins.empty())
	{
		//?P??X?L????z?E
		//?P ??X?L????z?E
		const auto& skin = loadedModelData.skins[0]; // ?L?????N?^?[?p??P ?skin
		m_SkinJoints = skin.joints;

		for (size_t i = 0; i < skin.joints.size(); ++i)
		{
			int nodeIdx = skin.joints[i]; // skin   i ???{ [     w   A S m [ h(m_Bones) ?  ?C   f b N X

			// GLTF ?s  ? D  (column-major) ???AXMLoadFloat4x4 ????    I ?s D  (row-major) ??      B
			//    ?  ?A     ? Transpose    ??    ?  I
			m_Bones[nodeIdx]->inverseBindPose = DirectX::XMLoadFloat4x4(&skin.inverseBindMatrices[i]);
		}
	}

	//?`EE???o?b?t?@?EE
	CreateBoneBuffer();


	//----- ?}?e???A???EE -----
	//?}?e???A?????EE
	//    f   ?f B   N g   p X ??o
	std::string directory = "";
	size_t lastSlash = _Path.find_last_of("/\\");
	if (lastSlash != std::string::npos)
	{
		directory = _Path.substr(0, lastSlash + 1);
	}

	std::vector<UINT> registeredMaterialIndices;
	if (loadedModelData.materials.empty())
	{
		UINT defaultMat = RegisterMatarial(L"Assets/Texture/Sample1.jpg", { 1.0f, 1.0f, 1.0f, 1.0f });
		registeredMaterialIndices.push_back(defaultMat);
	}
	else
	{
		for (const auto& matData : loadedModelData.materials)
		{
			std::string texPathStr = "";
			if (!matData.baseColorTexturePath.empty())
			{
				if (matData.baseColorTexturePath.rfind("Assets/", 0) == 0 || matData.baseColorTexturePath.rfind("Assets\\", 0) == 0)
				{
					texPathStr = matData.baseColorTexturePath;
				}
				else
				{
					texPathStr = directory + matData.baseColorTexturePath;
				}
			}
			else
			{
				texPathStr = "Assets/Texture/Sample1.jpg";
			}

			std::wstring texPathW(texPathStr.begin(), texPathStr.end());
			DirectX::XMFLOAT4 color = {
				matData.baseColorFactor[0],
				matData.baseColorFactor[1],
				matData.baseColorFactor[2],
				matData.baseColorFactor[3]
			};

			UINT matNum = RegisterMatarial(texPathW, color);
			registeredMaterialIndices.push_back(matNum);
		}
	}

	for (int i = 0; i < loadedModelData.nodes.size(); ++i)
	{
		const auto& node = loadedModelData.nodes[i];

		if (node.meshIndex != -1)
		{
			const auto& mesh = loadedModelData.meshes[node.meshIndex];

			UINT matIdx = 0;
			if (mesh.materialIndex < registeredMaterialIndices.size())
			{
				matIdx = registeredMaterialIndices[mesh.materialIndex];
			}
			else if (!registeredMaterialIndices.empty())
			{
				matIdx = registeredMaterialIndices[0];
			}

			RegisterMesh(
				matIdx,
				mesh.vertices.data(),
				mesh.vertices.size(),
				mesh.indices.data(),
				mesh.indices.size()
			);
		}
	}

	
	//int index = 2;
	//RegisterMesh(mat_num, &(loadedModelData.meshes[index].vertices[0]), loadedModelData.meshes[index].vertices.size(),
	//	&(loadedModelData.meshes[index].indices[0]), loadedModelData.meshes[index].indices.size());
	//m_Meshes[0]->SetVertex(
	//	&(loadedModelData.meshes[index].vertices[0]), loadedModelData.meshes[0].vertices.size(),
	//	&(loadedModelData.meshes[index].indices[0]), loadedModelData.meshes[0].indices.size()
	//	);



	//??????????????AMesh,Material,Bone??`E E?^???EE????
	//MakeBones(?`EE?`EE?`E E?^?????EE) 
	//Skin???E  ??T??????ABone??C???`E  ?N?X????X?g???EE????

	

	//MakeMashes(?}?e???A????`E E?^?????EE)

	//MakeMaterials



}

void CModel::Init()
{

}

void CModel::Update(float deltaTime)
{
	if (deltaTime <= 0.0f) deltaTime = TimeManager::GetInstance().GetDeltaTime();
	UpdateAnimation(deltaTime);
	for (auto& mesh : m_Meshes)
	{
		mesh->Update();
	}
}

void CModel::Update()
{
	float dt = TimeManager::GetInstance().GetDeltaTime();
	Update(dt);
}

void CModel::Draw() 
{
	//?`EE????X?V
	UpdateBones();

	//static float time = 0.01f; // ?K???????
	//time += 0.01f;
	//DirectX::XMMATRIX rot = DirectX::XMMatrixRotationX(time);
	//m_Bones[0]->localPose = rot;

	// GPU??`EE???EE????
	UpdateBoneBuffer();

	ID3D12GraphicsCommandList* commandList =
		DX12Manager::GetInstance().GetCommandList();

	
	ID3D12DescriptorHeap* heaps[] =
	{
		DX12Manager::GetInstance().GetSRVHeap()
	};
	commandList->SetDescriptorHeaps(1, heaps);

	CTransform* transform = m_Owner->GetComponent<CTransform>();

	//Mesh??`??
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

//?t?@?C???`E E?^????????????O??
//?F  ????????L?????N?^?[??p??????????A??????i??VE  ??EE  ????????E
UINT CModel::RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color)
{
	//Material??Vector???  
	//??????`E  ?X?`??????????s??
	m_Materials.push_back(std::make_shared<CMaterial>(_FilePath, _Color));

	//LastIndex???????E  ??E?L???`E  ????????EE?b??????)
	return m_Materials.size() - 1;
}

void CModel::UpdateAnimation(float deltaTime)
{
    if (m_currentAnimationIndex < 0 || m_currentAnimationIndex >= m_Animations.size() || m_Bones.empty()) return;

    m_animationTime += deltaTime;
    const AnimationData& anim = m_Animations[m_currentAnimationIndex];

    // Find max duration across all channels
    float maxDuration = 0.0f;
    for (const auto& channel : anim.channels) {
        if (channel.samplerIndex >= 0 && channel.samplerIndex < anim.samplers.size()) {
            const auto& sampler = anim.samplers[channel.samplerIndex];
            if (!sampler.input.empty()) {
                maxDuration = (std::max)(maxDuration, sampler.input.back());
            }
        }
    }

    if (!m_isLoop && maxDuration > 0.0f && m_animationTime >= maxDuration) {
        m_isAnimationFinished = true;
    }

    // Evaluate all channels
    for (const auto& channel : anim.channels) {
        if (channel.targetNodeIndex < 0 || channel.targetNodeIndex >= m_Bones.size()) continue;

        const AnimationSamplerData& sampler = anim.samplers[channel.samplerIndex];
        if (sampler.input.empty()) continue;

        float maxTime = sampler.input.back();
        float localTime = 0.0f;
        if (m_isLoop) {
            localTime = (maxTime > 0.0f) ? fmodf(m_animationTime, maxTime) : 0.0f;
        } else {
            localTime = (std::min)(m_animationTime, maxTime);
        }

        // Find keyframe
        size_t frameIdx = 0;
        for (size_t i = 0; i < sampler.input.size() - 1; ++i) {
            if (localTime >= sampler.input[i] && localTime < sampler.input[i + 1]) {
                frameIdx = i;
                break;
            }
        }
        if (localTime >= sampler.input.back()) {
            frameIdx = sampler.input.size() - 1;
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

