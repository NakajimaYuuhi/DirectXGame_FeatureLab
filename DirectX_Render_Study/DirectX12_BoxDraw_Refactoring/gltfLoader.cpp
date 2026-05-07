
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
void TestLoadGLTF()
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;

    std::string warn;
    std::string err;

    // 読みたい glb/gltf ファイル名
    std::string filename = "Assets/Model/OffensiveIdle.glb";

    bool ret = false;


    //モデルの情報
	LoadedModelData loadedModelData;



	//----- 読み込み -----
    if (filename.ends_with(".glb")) {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    }
    else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    }

	//----- 読み込み失敗 -----
    if (!warn.empty()) {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cout << "Err: " << err << std::endl;
    }

    if (!ret) {
        std::cout << "Failed to load: " << filename << std::endl;
        return ;
    }

	//----- 読み込み成功 -----
    std::cout << "Success! Loaded: " << filename << std::endl;

    //----- デバッグテスト -----
    std::cout << "Meshes:   " << model.meshes.size() << std::endl;
    std::cout << "Nodes:    " << model.nodes.size() << std::endl;
    std::cout << "Buffers:  " << model.buffers.size() << std::endl;
    std::cout << "Skins:    " << model.skins.size() << std::endl;
    std::cout << "Anims:    " << model.animations.size() << std::endl;

	//----- メッシュの頂点データを取得してみる -----
	for (const auto& mesh : model.meshes) // メッシュごとにループ
    {
    
		//== 変数宣言 ==
		MeshData meshData;                  //メッシュの情報
		std::vector<MeshVertex> vertices;   //頂点データのVector

        //----- 1.名前の取得 -----
		meshData.name = mesh.name;

		//----- 2.頂点データの取得 -----

		// メッシュのプリミティブごとにループ
        for (const auto& primitive : mesh.primitives) 
        {
            //----- 下準備 -----

            // 頂点属性を取得
            const auto& attributes = primitive.attributes;        

            //頂点数を取得
            const tinygltf::Accessor& posAccessor = model.accessors[attributes.at("POSITION")];
            size_t VertexCount = posAccessor.count;

			//----- Vectorの準備 -----
            //MeshのVector
			std::vector<MeshVertex> vertices;   //頂点のデータを格納するベクター
            vertices.resize(VertexCount);       //Resizeをしておく

            
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


                //頂点数の取得
                size_t vertexCount = accessor.count;



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

			//一旦Sizeを表示してみる
			std::cout << "Vertex Count: " << VertexCount << std::endl;

        }
	}

}