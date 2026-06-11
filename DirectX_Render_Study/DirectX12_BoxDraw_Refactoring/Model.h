//Model.h
//Model関連のクラス
//Mesh,Material,Nodeを持つ
//一旦作っただけ

//===== インクルード =====
//インクルードガード
#pragma once

//親クラス
#include "Component.h"

//基本機能
#include "StringAlias.h"	//文字列
#include "ContainerAlias.h"	//コンテナ
#include "SmartPtrAlias.h"	//スマートポインタ

//Mesh
#include "Mesh.h"
//Material
#include "Material.h"
//Bone
#include "Bone.h"


//===== エイリアス =====
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

//===== 前方宣言 =====


//===== クラス定義 =====
class CModel : public CComponent
{
public:
	CModel();

	void Init();
	void Update();
	void Draw();

	void SetBlendMode(BlendMode mode) { m_BlendMode = mode; }

	//板ポリとかなら、これを使う
	//LoadModel(こっちの方がいいかも)
	//void RegisterMesh(UINT _MatIdx, Primitive _Primitive);
	
	//一旦立方体がデフォルト
	//頂点情報、インデックス情報も渡せるようになるとグッド
	void RegisterMesh(UINT _MatIdx);
	void RegisterMesh(UINT _MatIdx, const MeshVertex* vertices, size_t vertexCount,
		const uint32_t* indices, size_t indexCount);

	//頂点情報、インデックス情報のセット
	
	UINT RegisterMatarial(wstring _FilePath,DirectX::XMFLOAT4 _Color);

	void CalculateRecursive(int index);

	//ボーンの更新
	void UpdateBones();


	void CreateTmpBoneData();

	void CreateBoneBuffer();
	void UpdateBoneBuffer();    // CPU→GPUへの転送


	//Modelのロード
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

		// For animated models, bones should be cloned per instance, but for static models copying is fine.
		m_Bones = other->m_Bones;
		m_SkinningMatrices = other->m_SkinningMatrices;
		m_BoneBuffer = other->m_BoneBuffer;
		m_BoneSrvGpuHandle = other->m_BoneSrvGpuHandle;
        
        m_Animations = other->m_Animations;
        m_SkinJoints = other->m_SkinJoints;
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
	//一旦Mesh1つだけ
	Meshes m_Meshes;

	// 各メッシュが使うマテリアルのインデックス
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

