//Model.h
//Modelﾖ連ﾌクX
//Mesh,Material,Node
//U

//===== CN[h =====
//CN[hK[h
#pragma once

//eNX
#include "Component.h"

//{@\
#include "StringAlias.h"	//
#include "ContainerAlias.h"	//Rei
#include "SmartPtrAlias.h"	//X}[g|C^

//Mesh
#include "Mesh.h"
//Material
#include "Material.h"
//Bone
#include "Bone.h"


//===== GCAX =====
//Mesh
using Mesh = SharedPtr<CMesh>;
using Meshes = Vector<Mesh>;

//Material
using Material = SharedPtr<CMaterial>;
using Materials = Vector<Material>;

//Bone
using Bone = SharedPtr<CBone>;
using Bones = Vector<Bone>;

//SkinningMatrix
using SkinningMatrix = DirectX::XMMATRIX;
using SkinningMatrices = Vector<SkinningMatrix>;

//===== O骭ｾ =====


//===== NX` =====
class CModel : public CComponent
{
public:
	CModel();

	void Init();
	void Update();
	void Draw();

	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }

	//ﾂポﾆな・g
	//LoadModel(ﾌ・
	//void RegisterMesh(UINT _MatIdx, Primitive _Primitive);
	
	//UﾌＧtHg
	//_ACfbNXn謔､ﾉなとグbh
	void RegisterMesh(UINT _MatIdx);
	void RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount,
		const uint32_t* indices, size_t indexCount);

	//_ACfbNXﾌセbg
	
	UINT RegisterMatarial(wstring _FilePath,DirectX::XMFLOAT4 _Color);

	void CalculateRecursive(int index);

	//{[ﾌ更V
	void UpdateBones();


	void CreateTmpBoneData();

	void CreateBoneBuffer();
	void UpdateBoneBuffer();    // CPUGPUﾖの転


	//Modelﾌゼh
	void ModelLoad(std::string _Path);

	void CopyFrom(const std::shared_ptr<CModel>& other)
	{
		m_Meshes = other->m_Meshes;
		
		// Deep copy materials so each instance can have its own textures
		m_Materials.clear();
		for (auto& mat : other->m_Materials) {
			if (mat) {
				m_Materials.push_back(std::make_shared<CMaterial>(*mat));
			} else {
				m_Materials.push_back(nullptr);
			}
		}
		
		m_MeshMaterialIndices = other->m_MeshMaterialIndices;
		m_BlendMode = other->m_BlendMode;

		m_Animations = other->m_Animations;
		m_SkinJoints = other->m_SkinJoints;

		// 繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ逕ｨ縺ｫ繝懊・繝ｳ繧偵ョ繧｣繝ｼ繝励さ繝斐・・亥・譛峨＠縺ｪ縺・ｼ・
		m_Bones.clear();
		for (const auto& otherBone : other->m_Bones)
		{
			auto bone = std::make_shared<CBone>();
			bone->name = otherBone->name;
			bone->parentIndex = otherBone->parentIndex;
			bone->children = otherBone->children;

			bone->scale = otherBone->scale;
			bone->rotation = otherBone->rotation;
			bone->translation = otherBone->translation;
			bone->localPose = otherBone->localPose;
			bone->globalPose = otherBone->globalPose;
			bone->localBindPose = otherBone->localBindPose;
			bone->globalBindPose = otherBone->globalBindPose;
			bone->inverseBindPose = otherBone->inverseBindPose;

			m_Bones.push_back(bone);
		}

		m_SkinningMatrices = other->m_SkinningMatrices;

		// 蝗ｺ譛峨・SRV縺ｨ繝舌ャ繝輔ぃ繧堤函謌舌☆繧・
		CreateBoneBuffer();
	}

	void SetMaterialTexture(const std::wstring& texturePath, UINT materialIndex = 0)
	{
		if (materialIndex < m_Materials.size() && m_Materials[materialIndex])
		{
			m_Materials[materialIndex]->LoadTexture(texturePath);
		}
	}

	void PlayAnimation(int index) {
		if (index >= 0 && index < m_Animations.size()) {
			m_currentAnimationIndex = index;
			m_animationTime = 0.0f;
		}
	}

	void UpdateAnimation(float deltaTime);

private:
	//Mesh
	//UMesh1ﾂ・
	Meshes m_Meshes;

	// ebVg}eAﾌイfbNX
	std::vector<UINT> m_MeshMaterialIndices;

	BlendMode m_BlendMode = BlendMode::Opaque;

	//Material
	Materials m_Materials;

	//Bone
	Bones m_Bones;

	//SkinningMatrix
	SkinningMatrices m_SkinningMatrices;

	ComPtr<ID3D12Resource> m_BoneBuffer;     // StructuredBuffer
	D3D12_GPU_DESCRIPTOR_HANDLE m_BoneSrvGpuHandle{};
	UINT m_boneSrvIndex = 0;

	std::vector<AnimationData> m_Animations;
	std::vector<int> m_SkinJoints;
	int m_currentAnimationIndex = -1;
	float m_animationTime = 0.0f;
};

