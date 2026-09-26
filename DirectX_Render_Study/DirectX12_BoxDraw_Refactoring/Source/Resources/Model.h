#pragma once

#include "Component.h"
#include "RenderLayer.h"
#include "StringAlias.h"
#include "ContainerAlias.h"
#include "SmartPtrAlias.h"
#include "Mesh.h"
#include "Material.h"
#include "Bone.h"
#include "TimeManager.h"
#include <windows.h>
#include <memory>
#include <vector>
#include <string>

// Type aliases
using Mesh = SharedPtr<CMesh>;
using Meshes = Vector<Mesh>;

using Material = SharedPtr<CMaterial>;
using Materials = Vector<Material>;

using Bone = SharedPtr<CBone>;
using Bones = Vector<Bone>;

using SkinningMatrix = DirectX::XMMATRIX;
using SkinningMatrices = Vector<SkinningMatrix>;

class CModel : public CComponent
{
public:
	CModel();
	~CModel();

	void Init() override;
	void Update(float deltaTime) override;
	void Update();
	void Draw() override;

	UpdatePhase GetUpdatePhase() const override { return UpdatePhase::Animation; }

	// Render layer
	RenderLayer GetRenderLayer() const { return m_renderLayer; }
	void SetRenderLayer(RenderLayer layer) { m_renderLayer = layer; }

	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }

	void SetBlendModeAll(BlendMode mode)
	{
		for (auto& mat : m_Materials)
		{
			if (mat) mat->SetBlendMode(mode);
		}
	}

	void RegisterMesh(UINT _MatIdx);
	void RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount,
		const uint32_t* indices, size_t indexCount);

	UINT RegisterMatarial(wstring _FilePath, DirectX::XMFLOAT4 _Color);

	void CalculateRecursive(int index);

	void UpdateBones();
	void CreateTmpBoneData();
	void CreateBoneBuffer();
	void UpdateBoneBuffer();

	void ModelLoad(std::string _Path);

	void CopyFrom(const std::shared_ptr<CModel>& other)
	{
		m_Meshes = other->m_Meshes;

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
		m_renderLayer = other->m_renderLayer;

		m_Animations = other->m_Animations;
		m_SkinJoints = other->m_SkinJoints;

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
		m_modelPath = other->m_modelPath;

		CreateBoneBuffer();
	}

	const std::string& GetModelPath() const { return m_modelPath; }
	void SetModelPath(const std::string& path) { m_modelPath = path; }

	void SetMaterialTexture(const std::wstring& texturePath, UINT materialIndex = 0)
	{
		if (materialIndex < m_Materials.size() && m_Materials[materialIndex])
		{
			m_Materials[materialIndex]->LoadTexture(texturePath);
		}
	}

	void SetShaderAll(const std::wstring& shaderFile, const std::string& vsEntry = "VSMain", const std::string& psEntry = "PSMain")
	{
		for (auto& mat : m_Materials)
		{
			if (mat)
			{
				mat->SetShader(shaderFile, vsEntry, psEntry);
			}
		}
	}

	void SetShader(UINT materialIndex, const std::wstring& shaderFile, const std::string& vsEntry = "VSMain", const std::string& psEntry = "PSMain")
	{
		if (materialIndex < m_Materials.size() && m_Materials[materialIndex])
		{
			m_Materials[materialIndex]->SetShader(shaderFile, vsEntry, psEntry);
		}
	}

	void PlayAnimation(int index, bool isLoop = true) {
		if (index >= 0 && index < m_Animations.size()) {
			m_currentAnimationIndex = index;
			m_animationTime = 0.0f;
			m_isLoop = isLoop;
			m_isAnimationFinished = false;
		}
	}

	void PlayAnimation(const std::string& name, bool isLoop = true) {
		for (size_t i = 0; i < m_Animations.size(); ++i) {
			if (m_Animations[i].name == name) {
				PlayAnimation(static_cast<int>(i), isLoop);
				return;
			}
		}

		if (!m_Animations.empty()) {
			PlayAnimation(0, isLoop);
			return;
		}

#ifdef _DEBUG
		std::string msg = "[Model] Animation not found: " + name + "\n";
		OutputDebugStringA(msg.c_str());
#endif
	}

	bool IsAnimationFinished() const { return m_isAnimationFinished; }
	bool IsAnimationLooping() const { return m_isLoop; }
	float GetAnimationTime() const { return m_animationTime; }

	void UpdateAnimation(float deltaTime);

private:
	Meshes m_Meshes;
	std::vector<UINT> m_MeshMaterialIndices;

	BlendMode m_BlendMode = BlendMode::Opaque;
	RenderLayer m_renderLayer = RenderLayer::Opaque;

	Materials m_Materials;
	Bones m_Bones;
	SkinningMatrices m_SkinningMatrices;

	ComPtr<ID3D12Resource> m_BoneBuffer;
	D3D12_CPU_DESCRIPTOR_HANDLE m_BoneSrvCpuHandle{0};
	D3D12_GPU_DESCRIPTOR_HANDLE m_BoneSrvGpuHandle{0};

	std::vector<AnimationData> m_Animations;
	std::vector<int> m_SkinJoints;
	int m_currentAnimationIndex = -1;
	float m_animationTime = 0.0f;
	bool m_isLoop = true;
	bool m_isAnimationFinished = false;
	std::string m_modelPath;
};
