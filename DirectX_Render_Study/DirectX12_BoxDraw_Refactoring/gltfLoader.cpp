
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tiny_gltf.h"
#include <string>
#include <iostream>

#include <vector>

#include "gltfLoader.h"

//構造体情報
#include "ModelData.h"



//読み込み成功したっぽい
LoadedModelData TestLoadGLTF()
{

    //----- 変数宣言 -----
    // GLTF関連
    //< ロードに必要なもの >
    tinygltf::TinyGLTF loader;  //コンテキスト
    tinygltf::Model model;      //読み込んだデータを格納

    //< エラーチェック用 >
    bool loadResult;
    std::string warn;
    std::string err;

    //< ファイル名 >
    //読みたい glb/gltf ファイル名
    std::string filename = "Assets/Model/OffensiveIdle.glb";



    //モデルの情報
	LoadedModelData loadedModelData;


    //----- 変数の初期化 -----
    loadResult = false;


	//----- 読み込み -----
    if (filename.ends_with(".glb")) {
        loadResult = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    }
    else {
        loadResult = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    }

	//----- 読み込み失敗 -----
    if (!warn.empty()) {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "Err: " << err << std::endl;
    }

    if (!loadResult) {
        std::cout << "Failed to load: " << filename << std::endl;
        //return ;
    }

	//----- 読み込み成功 -----
    std::cout << "Success! Loaded: " << filename << std::endl;

    //----- デバッグテスト -----
    std::cout << "Meshes:   " << model.meshes.size() << std::endl;
    std::cout << "Nodes:    " << model.nodes.size() << std::endl;
    std::cout << "Buffers:  " << model.buffers.size() << std::endl;
    std::cout << "Skins:    " << model.skins.size() << std::endl;
    std::cout << "Anims:    " << model.animations.size() << std::endl;


	//----- メッシュデータを取得する -----
	for (const auto& mesh : model.meshes) // メッシュごとにループ
    {
    
		//== 変数宣言 ==
        //GLTFはMesh -> Primitiveの構造
        //Primitiveをメッシュとして取り込むので、ベースになるものを宣言しておく
        std::string meshName;   //メッシュの名前
        int PrimitiveNum;       //プリミティブのインデックス

        //== 変数の初期化 ==
        meshName = mesh.name;   //名前を取得
        PrimitiveNum = 0;       


		//== 頂点データの取得 ==
		// メッシュのプリミティブごとにループ
        for (const auto& primitive : mesh.primitives) 
        {
            //== 変数宣言 ==
            std::string primitiveName;
            MeshData meshData;                  //メッシュの情報
            std::vector<MeshVertex> vertices;   //頂点データのVector

            // 頂点属性を取得
            const auto& attributes = primitive.attributes;        

            //頂点数を取得
            const tinygltf::Accessor& posAccessor = model.accessors[attributes.at("POSITION")];
            size_t VertexCount = posAccessor.count;


			//----- Vectorの準備 -----
            //Vectorのサイズを直す
            vertices.resize(VertexCount);       //Resizeをしておく

            //== verticesにデータを入れる ==
            
            //----- 位置データを取得 -----
            if (attributes.find("POSITION") != attributes.end()) 
            {
                //TODO:各情報を理解する

				//頂点属性からアクセサーを取得
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("POSITION")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];


				//----- 頂点データの取得 -----
                // バッファから頂点データを読み取る
                //float3配列に変換
				//x,y,zの順番で格納される
                const float* vertexData = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

				//頂点データの例: 位置データをfloat3配列として取得
                const float* positions =
                    reinterpret_cast<const float*>(
                        &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                        );


				//----- 頂点データの格納 -----

				//頂点データをMeshVertex構造体のVectorに格納
                for (size_t i = 0; i < VertexCount; i++)
                {
                    vertices[i].position[0] = positions[i * 3 + 0];
                    vertices[i].position[1] = positions[i * 3 + 1];
                    vertices[i].position[2] = positions[i * 3 + 2];
                }

            }

			//----- 法線データを取得 -----
            if (attributes.find("NORMAL") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* normals =
                    reinterpret_cast<const float*>(
                        &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                        );

                for (size_t i = 0; i < VertexCount; i++)
                {
                    vertices[i].normal[0] = normals[i * 3 + 0];
                    vertices[i].normal[1] = normals[i * 3 + 1];
                    vertices[i].normal[2] = normals[i * 3 + 2];
                }

            }

			//----- UVデータを取得 -----
            if (attributes.find("TEXCOORD_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
                const float* uvs =
                    reinterpret_cast<const float*>(
                        &buffer.data[bufferView.byteOffset + accessor.byteOffset]
                        );
                for (size_t i = 0; i < VertexCount; i++)
                {
                    vertices[i].uv[0] = uvs[i * 2 + 0];
                    vertices[i].uv[1] = uvs[i * 2 + 1];
                }
            }

            //----- ボーンの情報 -----
            if (attributes.find("JOINTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("JOINTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned short* joints =
                    reinterpret_cast<const unsigned short*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    vertices[i].boneIndices[0] = joints[i * 4 + 0];
                    vertices[i].boneIndices[1] = joints[i * 4 + 1];
                    vertices[i].boneIndices[2] = joints[i * 4 + 2];
                    vertices[i].boneIndices[3] = joints[i * 4 + 3];
                }
            }

            //----- ウエイト -----
            if (attributes.find("WEIGHTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("WEIGHTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const float* weights =
                    reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    vertices[i].boneWeights[0] = weights[i * 4 + 0];
                    vertices[i].boneWeights[1] = weights[i * 4 + 1];
                    vertices[i].boneWeights[2] = weights[i * 4 + 2];
                    vertices[i].boneWeights[3] = weights[i * 4 + 3];
                }
            }

            //----- マテリアルの情報 -----
            int materialIndex = primitive.material;
            meshData.materialIndex = materialIndex;


            //名前を入れる
            primitiveName = meshName;
            primitiveName += "_Primitive";
            primitiveName += std::to_string(PrimitiveNum);

            meshData.name = primitiveName;

            //verticesのvectorを格納する
            meshData.vertices = vertices;




            //== Indexの情報を読み取る
            std::vector<uint32_t> indices;

            if (primitive.indices < 0) {
                continue;
            }

            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            // componentType によって読み方が変わる
            switch (accessor.componentType)
            {
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            {
                const uint16_t* buf = reinterpret_cast<const uint16_t*>(dataPtr);
                for (size_t i = 0; i < accessor.count; i++) {
                    indices.push_back(static_cast<uint32_t>(buf[i]));
                }
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            {
                const uint32_t* buf = reinterpret_cast<const uint32_t*>(dataPtr);
                for (size_t i = 0; i < accessor.count; i++) {
                    indices.push_back(buf[i]);
                }
                break;
            }
            case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            {
                const uint8_t* buf = reinterpret_cast<const uint8_t*>(dataPtr);
                for (size_t i = 0; i < accessor.count; i++) {
                    indices.push_back(static_cast<uint32_t>(buf[i]));
                }
                break;
            }
            default:
                std::cerr << "Unsupported index component type\n";
                break;
            }

            // 読み込んだ indices を格納
            meshData.indices.assign(std::begin(indices), std::end(indices));


            //loadModelDataに格納
            loadedModelData.meshes.push_back(meshData);
    


            //Primitiveのインデックスのカウント
            PrimitiveNum++;
        }
	}

    //----- マテリアルのデータの取得 -----
    for (const auto& material : model.materials)//マテリアルごとに実行
    {
        MaterialData materialData;

        // 名前
        materialData.name = material.name;

        // PBR メイン情報
        const auto& pbr = material.pbrMetallicRoughness;

        // BaseColorFactor
        if (!pbr.baseColorFactor.empty())
        {
            materialData.baseColorFactor[0] = pbr.baseColorFactor[0];
            materialData.baseColorFactor[1] = pbr.baseColorFactor[1];
            materialData.baseColorFactor[2] = pbr.baseColorFactor[2];
            materialData.baseColorFactor[3] = pbr.baseColorFactor[3];
        }

        // Metallic / Roughness
        materialData.metallicFactor = pbr.metallicFactor;
        materialData.roughnessFactor = pbr.roughnessFactor;


        //---------- BaseColorTexture --------------
        if (pbr.baseColorTexture.index >= 0)
        {
            int texIndex = pbr.baseColorTexture.index;

            const tinygltf::Texture& tex = model.textures[texIndex];

            if (tex.source >= 0)
            {
                const tinygltf::Image& image = model.images[tex.source];
                materialData.baseColorTexturePath = image.uri;     // 画像ファイル名
            }
        }

        //---------- MetallicRoughnessTexture --------------
        if (pbr.metallicRoughnessTexture.index >= 0)
        {
            int texIndex = pbr.metallicRoughnessTexture.index;

            const tinygltf::Texture& tex = model.textures[texIndex];

            if (tex.source >= 0)
            {
                const tinygltf::Image& image = model.images[tex.source];
                materialData.metallicRoughnessTexturePath = image.uri;
            }
        }

        // LoadedModelData に追加
        loadedModelData.materials.push_back(materialData);
    }

    //----- ノードのデータの取得 -----
    for (const auto& node : model.nodes)
    {
        NodeData nodeData;

        //--- 名前 ---
        nodeData.name = node.name;

        //--- メッシュ参照 ---
        nodeData.meshIndex = node.mesh;  // -1 の場合は mesh なし

        //--- 子ノード ---
        nodeData.children = node.children;

        //--- TRS or Matrix ---

        // Translation
        if (!node.translation.empty()) {
            nodeData.translation[0] = node.translation[0];
            nodeData.translation[1] = node.translation[1];
            nodeData.translation[2] = node.translation[2];
        }

        // Rotation (Quaternion)
        if (!node.rotation.empty()) {
            nodeData.rotation[0] = node.rotation[0];
            nodeData.rotation[1] = node.rotation[1];
            nodeData.rotation[2] = node.rotation[2];
            nodeData.rotation[3] = node.rotation[3];
        }

        // Scale
        if (!node.scale.empty()) {
            nodeData.scale[0] = node.scale[0];
            nodeData.scale[1] = node.scale[1];
            nodeData.scale[2] = node.scale[2];
        }

        // Matrix（4x4行列）
        if (!node.matrix.empty()) 
        {
            for (int i = 0; i < 16; i++) {
                nodeData.matrix[i] = node.matrix[i];
            }
        }
        else 
        {
            // TRS → 行列変換が必要ならここでする
            // 今は省略可能
        }

        //--- Skin index（スケルトン） ---
        nodeData.skinIndex = node.skin;

        //→ LoadedModelData に追加
        loadedModelData.nodes.push_back(nodeData);
    }

    //----- スキンのデータの取得 -----
    for (const auto& skin : model.skins)
    {
        SkinData skinData;

        //jointsを入れる
        skinData.joints = skin.joints;

        //
        if (skin.inverseBindMatrices >= 0) {
            // bufferView から inverseBindMatrices を読む
            const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

            size_t count = accessor.count;
            skinData.inverseBindMatrices.resize(count);

            for (size_t i = 0; i < count; ++i) {
                const float* m = reinterpret_cast<const float*>(dataPtr + accessor.ByteStride(bufferView) * i);

                // glTF は列優先(column-major)
                DirectX::XMFLOAT4X4 mat;
                memcpy(&mat, m, sizeof(float) * 16);

                skinData.inverseBindMatrices[i] = mat;
            }
        }

        loadedModelData.skins.push_back(skinData);
    }
    

    //値を返す
    return loadedModelData;
}