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


//===== エイリアス =====
//Mesh
using Mesh = SharedPtr<CMesh>;
using Meshes = Vector<Mesh>;

//Material
using Material = SharedPtr<CMaterial>;
using Materials = Vector<Material>;

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

	//頂点情報、インデックス情報のセット
	
	UINT RegisterMatarial(wstring _FilePath,DirectX::XMFLOAT4 _Color);

private:
	//Mesh
	//一旦Mesh1つだけ
	Mesh m_Mesh;
	Meshes m_Meshes;

	//Meterial
	Materials m_Materials;

	//Node
};

