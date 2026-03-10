#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Model::MakeMesh(const void* ptr)
{
	// 事前準備
	aiVector3D zero3(0.0f, 0.0f, 0.0f);
	aiColor4D one4(1.0f, 1.0f, 1.0f, 1.0f);
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);
	bool isXFlip = (m_loadFlip == Flip::XFlip || m_loadFlip == Flip::XZFlip);
	bool isIdxFlip = (m_loadFlip == Flip::XFlip || m_loadFlip == Flip::ZFlip);
	float xFlip = isXFlip ? -1.0f : 1.0f;
	int idx1 = isIdxFlip ? 2 : 1;
	int idx2 = isIdxFlip ? 1 : 2;

	// メッシュの作成
	m_meshes.resize(pScene->mNumMeshes);
	for (unsigned int i = 0; i < m_meshes.size(); ++i)
	{
		aiMesh* assimpMesh = pScene->mMeshes[i];
		Mesh& mesh = m_meshes[i];

		// ノード階層内の探索
		mesh.nodeIndex = FindNode(assimpMesh->mName.data);

		// モーフを行ってないモデルはシーンにメッシュが登録されていないので
		// 特にエラー処理を行わないようにする
		// if (mesh.nodeIndex == NODE_NONE) { continue; }

		// 頂点書き込み先の領域を用意
		mesh.vertices.resize(assimpMesh->mNumVertices);

		// 頂点データの書き込み
		for (unsigned int j = 0; j < mesh.vertices.size(); ++j) {
			// モデルデータから値の取得
			aiVector3D pos = assimpMesh->mVertices[j];
			aiVector3D normal = assimpMesh->HasNormals() ? assimpMesh->mNormals[j] : zero3;
			aiVector3D uv = assimpMesh->HasTextureCoords(0) ?
				assimpMesh->mTextureCoords[0][j] : zero3;
			aiColor4D color = assimpMesh->HasVertexColors(0) ? assimpMesh->mColors[0][j] : one4;
			// 値を設定
			mesh.vertices[j] = {
				DirectX::XMFLOAT3(pos.x * m_loadScale * xFlip, pos.y * m_loadScale, pos.z * m_loadScale),
				DirectX::XMFLOAT3(normal.x * xFlip, normal.y, normal.z),
				DirectX::XMFLOAT2(uv.x, uv.y),
				DirectX::XMFLOAT4(color.r, color.g, color.b, color.a)
			};
		}

		// ボーン生成
		MakeVertexWeight(pScene, i);

		// インデックスの書き込み先の用意
		// mNumFacesはポリゴンの数を表す(１ポリゴンで3インデックス
		mesh.indices.resize(assimpMesh->mNumFaces * 3);

		// インデックスの書き込み
		for (unsigned int j = 0; j < assimpMesh->mNumFaces; ++j) {
			// モデルデータから値の取得
			aiFace face = assimpMesh->mFaces[j];

			// 値の設定
			int idx = j * 3;
			mesh.indices[idx + 0] = face.mIndices[0];
			mesh.indices[idx + 1] = face.mIndices[idx1];
			mesh.indices[idx + 2] = face.mIndices[idx2];
		}

		// マテリアルの割り当て
		mesh.materialID = assimpMesh->mMaterialIndex;

		// メッシュを元に頂点バッファ作成
		MeshBuffer::Description desc = {};
		desc.pVtx = mesh.vertices.data();
		desc.vtxSize = sizeof(Vertex);
		desc.vtxCount = static_cast<UINT>(mesh.vertices.size());
		desc.pIdx = mesh.indices.data();
		desc.idxSize = sizeof(unsigned long);
		desc.idxCount = static_cast<UINT>(mesh.indices.size());
		desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		mesh.pMesh = new MeshBuffer();
		mesh.pMesh->Create(desc);
	}
}

void Model::MakeMaterial(const void* ptr, std::string directory)
{
	// 事前準備
	aiColor3D color(0.0f, 0.0f, 0.0f);
	const aiScene* pScene = reinterpret_cast<const aiScene*>(ptr);
	float shininess;

	// マテリアルの作成
	m_materials.resize(pScene->mNumMaterials);
	for (unsigned int i = 0; i < m_materials.size(); ++i)
	{
		aiMaterial* assimpMaterial = pScene->mMaterials[i];
		Material& material = m_materials[i];

		//--- 各種マテリアルパラメーターの読み取り
		// 拡散光の読み取り
		if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
			material.diffuse = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		else
			material.diffuse = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		// 環境光の読み取り
		if (assimpMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
			material.ambient = DirectX::XMFLOAT4(color.r, color.g, color.b, 1.0f);
		else
			material.ambient = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		// 反射光の読み取り
		if (assimpMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
			material.specular = DirectX::XMFLOAT4(color.r, color.g, color.b, 0.0f);
		else
			material.specular = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		// 反射光の強さを読み取り
		if (assimpMaterial->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
			material.specular.w = shininess;

		// テクスチャ読み込み処理
		HRESULT hr;
		aiString path;

		// テクスチャのパス情報を読み込み
		material.pTexture = nullptr;
		if (assimpMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) != AI_SUCCESS) {
			continue;
		}

		// テクスチャ領域確保
		material.pTexture = new Texture;

		// そのまま読み込み
		hr = material.pTexture->Create(path.C_Str());
		if (SUCCEEDED(hr)) { continue; }

		// ディレクトリと連結して探索
		hr = material.pTexture->Create((directory + path.C_Str()).c_str());
		if (SUCCEEDED(hr)) { continue; }

		// モデルと同じ階層を探索
		// パスからファイル名のみ取得
		std::string fullPath = path.C_Str();
		std::string baseDir = GetDirectory(fullPath.c_str());
		std::string fileName = fullPath.substr(baseDir.size());

		// テクスチャの読込
		hr = material.pTexture->Create((directory + fileName).c_str());
		if (SUCCEEDED(hr)) { continue; }

		// テクスチャが見つからなかった
		delete material.pTexture;
		material.pTexture = nullptr;
		SetErrorMessage("Not find texture. [" + fullPath + "]");
	}
}