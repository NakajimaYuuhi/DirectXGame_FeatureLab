
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tiny_gltf.h"

#include <iostream>

#include <vector>

#include "gltfLoader.h"

//?\??????
#include "ModelData.h"



//??????????????????
LoadedModelData TestLoadGLTF(std::string _fileName)
{

    //----- ????? -----
    // GLTF??A
    //< ???[?h??K?v???? >
    tinygltf::TinyGLTF loader;  //?R???e?L?X?g
    tinygltf::Model model;      //???????f?[?^??i?[

    //< ?G???[?`?F?b?N?p >
    bool loadResult;
    std::string warn;
    std::string err;

    //< ?t?@?C???? >
    //?????? glb/gltf ?t?@?C????
    std::string filename = _fileName;
    //std::string filename = "Assets/Model/OffensiveIdle.glb";
    //std::string filename = "Assets/Model/cube.glb";



    //???f??????
	LoadedModelData loadedModelData;


    //----- ?????????? -----
    loadResult = false;


	//----- ?????? -----
    if (filename.ends_with(".glb")) {
        loadResult = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    }
    else {
        loadResult = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    }

	//----- ????????s -----
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

	//----- ????????? -----
    std::cout << "Success! Loaded: " << filename << std::endl;

    //----- ?f?o?b?O?e?X?g -----
    std::cout << "Meshes:   " << model.meshes.size() << std::endl;
    std::cout << "Nodes:    " << model.nodes.size() << std::endl;
    std::cout << "Buffers:  " << model.buffers.size() << std::endl;
    std::cout << "Skins:    " << model.skins.size() << std::endl;
    std::cout << "Anims:    " << model.animations.size() << std::endl;


	//----- ???b?V???f?[?^??èÔ???? -----
	for (const auto& mesh : model.meshes) // ???b?V?????????[?v
    {
    
		//== ????? ==
        //GLTF??Mesh -> Primitive??\??
        //Primitive????b?V?????????????A?x?[?X???????????????
        std::string meshName;   //???b?V??????O
        int PrimitiveNum;       //?v???~?e?B?u??C???f?b?N?X

        //== ?????????? ==
        meshName = mesh.name;   //???O??èÔ
        PrimitiveNum = 0;       


		//== ???_?f?[?^??èÔ ==
		// ???b?V????v???~?e?B?u???????[?v
        for (const auto& primitive : mesh.primitives) 
        {
            //== ????? ==
            std::string primitiveName;
            MeshData meshData;                  //???b?V??????
            std::vector<MeshVertex> vertices;   //???_?f?[?^??Vector

            // ???_??????èÔ
            const auto& attributes = primitive.attributes;        

            //???_????èÔ
            const tinygltf::Accessor& posAccessor = model.accessors[attributes.at("POSITION")];
            size_t VertexCount = posAccessor.count;


			//----- Vector????? -----
            //Vector??T?C?Y????
            vertices.resize(VertexCount);       //Resize????????

            // ?S?~?f?[?^????????h??????A?{?[??????E?F?C?g???????
            for (size_t i = 0; i < VertexCount; i++)
            {
                vertices[i].boneIndices[0] = 0;
                vertices[i].boneIndices[1] = 0;
                vertices[i].boneIndices[2] = 0;
                vertices[i].boneIndices[3] = 0;
                vertices[i].boneWeights[0] = 1.0f; // ?????{?[????E?F?C?g100%
                vertices[i].boneWeights[1] = 0.0f;
                vertices[i].boneWeights[2] = 0.0f;
                vertices[i].boneWeights[3] = 0.0f;
            }

            //== vertices??f?[?^?????? ==
            
            //----- POSITION -----
            if (attributes.find("POSITION") != attributes.end()) 
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("POSITION")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* pos = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].position[0] = pos[0];
                    vertices[i].position[1] = pos[1];
                    vertices[i].position[2] = pos[2];
                }
            }

			//----- NORMAL -----
            if (attributes.find("NORMAL") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("NORMAL")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* norm = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].normal[0] = norm[0];
                    vertices[i].normal[1] = norm[1];
                    vertices[i].normal[2] = norm[2];
                }
            }

			//----- TEXCOORD_0 -----
            if (attributes.find("TEXCOORD_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("TEXCOORD_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                for (size_t i = 0; i < VertexCount; i++)
                {
                    const float* uv = reinterpret_cast<const float*>(dataPtr + i * stride);
                    vertices[i].uv[0] = uv[0];
                    vertices[i].uv[1] = uv[1];
                }
            }

            //----- JOINTS_0 -----
            if (attributes.find("JOINTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("JOINTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) // 5123
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint16_t* joints = reinterpret_cast<const uint16_t*>(dataPtr + i * stride);
                        vertices[i].boneIndices[0] = static_cast<uint32_t>(joints[0]);
                        vertices[i].boneIndices[1] = static_cast<uint32_t>(joints[1]);
                        vertices[i].boneIndices[2] = static_cast<uint32_t>(joints[2]);
                        vertices[i].boneIndices[3] = static_cast<uint32_t>(joints[3]);
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) // 5121
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint8_t* joints = reinterpret_cast<const uint8_t*>(dataPtr + i * stride);
                        vertices[i].boneIndices[0] = static_cast<uint32_t>(joints[0]);
                        vertices[i].boneIndices[1] = static_cast<uint32_t>(joints[1]);
                        vertices[i].boneIndices[2] = static_cast<uint32_t>(joints[2]);
                        vertices[i].boneIndices[3] = static_cast<uint32_t>(joints[3]);
                    }
                }
            }

            //----- WEIGHTS_0 -----
            if (attributes.find("WEIGHTS_0") != attributes.end())
            {
                const tinygltf::Accessor& accessor = model.accessors[attributes.at("WEIGHTS_0")];
                const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

                const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
                size_t stride = accessor.ByteStride(bufferView);

                if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) // 5126
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const float* weights = reinterpret_cast<const float*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = weights[0];
                        vertices[i].boneWeights[1] = weights[1];
                        vertices[i].boneWeights[2] = weights[2];
                        vertices[i].boneWeights[3] = weights[3];
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) // 5123
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint16_t* weights = reinterpret_cast<const uint16_t*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = static_cast<float>(weights[0]) / 65535.0f;
                        vertices[i].boneWeights[1] = static_cast<float>(weights[1]) / 65535.0f;
                        vertices[i].boneWeights[2] = static_cast<float>(weights[2]) / 65535.0f;
                        vertices[i].boneWeights[3] = static_cast<float>(weights[3]) / 65535.0f;
                    }
                }
                else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) // 5121
                {
                    for (size_t i = 0; i < VertexCount; i++)
                    {
                        const uint8_t* weights = reinterpret_cast<const uint8_t*>(dataPtr + i * stride);
                        vertices[i].boneWeights[0] = static_cast<float>(weights[0]) / 255.0f;
                        vertices[i].boneWeights[1] = static_cast<float>(weights[1]) / 255.0f;
                        vertices[i].boneWeights[2] = static_cast<float>(weights[2]) / 255.0f;
                        vertices[i].boneWeights[3] = static_cast<float>(weights[3]) / 255.0f;
                    }
                }
                else
                {
                    std::cerr << "Unsupported WEIGHTS component type!\n";
                }
            }

            //----- ?}?e???A?????? -----
            int materialIndex = primitive.material;
            meshData.materialIndex = materialIndex;


            //???O??????
            primitiveName = meshName;
            primitiveName += "_Primitive";
            primitiveName += std::to_string(PrimitiveNum);

            meshData.name = primitiveName;

            //vertices??vector??i?[????
            meshData.vertices = vertices;




            //== Index?????????
            std::vector<uint32_t> indices;

            if (primitive.indices < 0) {
                continue;
            }

            const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            const unsigned char* dataPtr = buffer.data.data() + bufferView.byteOffset + accessor.byteOffset;

            // componentType ???????????????
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

            // ?????? indices ??i?[
            meshData.indices.assign(std::begin(indices), std::end(indices));


            //loadModelData??i?[
            loadedModelData.meshes.push_back(meshData);
    


            //Primitive??C???f?b?N?X??J?E???g
            PrimitiveNum++;
        }
	}

    //----- ?}?e???A????f?[?^??èÔ -----
    for (const auto& material : model.materials)//?}?e???A?????????s
    {
        MaterialData materialData;

        // ???O
        materialData.name = material.name;

        // PBR ???C?????
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
                if (!image.uri.empty())
                {
                    materialData.baseColorTexturePath = image.uri;
                }
                else if (!image.image.empty())
                {
                    std::string baseName = "embedded";
                    size_t lastSlash = _fileName.find_last_of("/\\");
                    if (lastSlash != std::string::npos)
                    {
                        baseName = _fileName.substr(lastSlash + 1);
                        size_t lastDot = baseName.find_last_of(".");
                        if (lastDot != std::string::npos)
                        {
                            baseName = baseName.substr(0, lastDot);
                        }
                    }
                    std::string outPath = "Assets/Texture/" + baseName + "_tex_" + std::to_string(tex.source) + ".png";
                    if (stbi_write_png(outPath.c_str(), image.width, image.height, image.component, image.image.data(), image.width * image.component))
                    {
                        materialData.baseColorTexturePath = outPath;
                    }
                }
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

        // LoadedModelData ????
        loadedModelData.materials.push_back(materialData);
    }

    //----- ?m?[?h??f?[?^??èÔ -----
    for (const auto& node : model.nodes)
    {
        NodeData nodeData;

        //--- ???O ---
        nodeData.name = node.name;

        //--- ???b?V???Q?? ---
        nodeData.meshIndex = node.mesh;  // -1 ????? mesh ???

        //--- ?q?m?[?h ---
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

        // Matrix?i4x4?s??j
        if (!node.matrix.empty()) 
        {
            for (int i = 0; i < 16; i++) {
                nodeData.matrix[i] = node.matrix[i];
            }
        }
        else 
        {
            // TRS ?? ?s???????K?v??????????
            // ????????\
        }

        //--- Skin index?i?X?P???g???j ---
        nodeData.skinIndex = node.skin;

        //?? LoadedModelData ????
        loadedModelData.nodes.push_back(nodeData);
    }

    //----- ?X?L????f?[?^??èÔ -----
    for (const auto& skin : model.skins)
    {
        SkinData skinData;

        //joints??????
        skinData.joints = skin.joints;

        //
        if (skin.inverseBindMatrices >= 0) {
            // bufferView ???? inverseBindMatrices ????
            const tinygltf::Accessor& accessor = model.accessors[skin.inverseBindMatrices];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

            const unsigned char* dataPtr = &buffer.data[bufferView.byteOffset + accessor.byteOffset];

            size_t count = accessor.count;
            skinData.inverseBindMatrices.resize(count);

            for (size_t i = 0; i < count; ++i) {
                const float* m = reinterpret_cast<const float*>(dataPtr + accessor.ByteStride(bufferView) * i);

                // glTF ???D??(column-major)
                DirectX::XMFLOAT4X4 mat;
                memcpy(&mat, m, sizeof(float) * 16);

                skinData.inverseBindMatrices[i] = mat;
            }
        }

        loadedModelData.skins.push_back(skinData);
    }
    
    // ----- Animations -----
    for (const auto& anim : model.animations) {
        AnimationData animData;
        animData.name = anim.name;

        // Parse samplers
        for (const auto& sampler : anim.samplers) {
            AnimationSamplerData samplerData;
            
            // get input accessor (time)
            const tinygltf::Accessor& inputAccessor = model.accessors[sampler.input];
            const tinygltf::BufferView& inputView = model.bufferViews[inputAccessor.bufferView];
            const tinygltf::Buffer& inputBuffer = model.buffers[inputView.buffer];
            const float* times = reinterpret_cast<const float*>(&inputBuffer.data[inputView.byteOffset + inputAccessor.byteOffset]);
            for(size_t i=0; i<inputAccessor.count; ++i) {
                samplerData.input.push_back(times[i]);
            }

            // get output accessor (values)
            const tinygltf::Accessor& outputAccessor = model.accessors[sampler.output];
            const tinygltf::BufferView& outputView = model.bufferViews[outputAccessor.bufferView];
            const tinygltf::Buffer& outputBuffer = model.buffers[outputView.buffer];
            const unsigned char* outputDataPtr = &outputBuffer.data[outputView.byteOffset + outputAccessor.byteOffset];
            int numComponents = tinygltf::GetNumComponentsInType(outputAccessor.type);
            size_t stride = outputAccessor.ByteStride(outputView);
            
            for(size_t i=0; i<outputAccessor.count; ++i) {
                std::vector<float> val(numComponents);
                const unsigned char* currentData = outputDataPtr + i * stride;
                
                for(int j=0; j<numComponents; ++j) {
                    if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                        val[j] = reinterpret_cast<const float*>(currentData)[j];
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
                        val[j] = std::max(reinterpret_cast<const int8_t*>(currentData)[j] / 127.0f, -1.0f);
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        val[j] = reinterpret_cast<const uint8_t*>(currentData)[j] / 255.0f;
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
                        val[j] = std::max(reinterpret_cast<const int16_t*>(currentData)[j] / 32767.0f, -1.0f);
                    }
                    else if (outputAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        val[j] = reinterpret_cast<const uint16_t*>(currentData)[j] / 65535.0f;
                    }
                }
                samplerData.output.push_back(val);
            }
            
            if (sampler.interpolation == "LINEAR") samplerData.interpolation = InterpolationType::LINEAR;
            else if (sampler.interpolation == "STEP") samplerData.interpolation = InterpolationType::STEP;
            else samplerData.interpolation = InterpolationType::CUBICSPLINE;

            animData.samplers.push_back(samplerData);
        }

        // Parse channels
        for (const auto& channel : anim.channels) {
            AnimationChannelData channelData;
            channelData.targetNodeIndex = channel.target_node;
            channelData.samplerIndex = channel.sampler;
            
            if (channel.target_path == "translation") channelData.path = AnimationPath::TRANSLATION;
            else if (channel.target_path == "rotation") channelData.path = AnimationPath::ROTATION;
            else if (channel.target_path == "scale") channelData.path = AnimationPath::SCALE;
            else channelData.path = AnimationPath::WEIGHTS;

            animData.channels.push_back(channelData);
        }

        loadedModelData.animations.push_back(animData);
    }
    OutputDebugString("");

    //?l????
    return loadedModelData;
}
