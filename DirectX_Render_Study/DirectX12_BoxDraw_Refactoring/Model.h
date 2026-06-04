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

private:
	//Mesh
	//一旦Mesh1つだけ
	Meshes m_Meshes;

	//Material
	Materials m_Materials;

	//Bone
	Bones m_Bones;

	//SkinningMatrix
	SkinningMatrices m_SkinningMatrices;

	ComPtr<ID3D12Resource> m_BoneBuffer;     // StructuredBuffer
	D3D12_GPU_DESCRIPTOR_HANDLE m_BoneSrvGpuHandle{};
	UINT m_boneSrvIndex = 0;

};

