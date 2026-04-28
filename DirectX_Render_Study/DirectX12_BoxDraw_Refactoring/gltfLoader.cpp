
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#define JSON_NOEXCEPTION
#include "tiny_gltf.h"
#include <string>
#include <iostream>



#include "gltfLoader.h"

void TestLoadGLTFv3()
{
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;

    std::string warn;
    std::string err;

    // 読みたい glb/gltf ファイル名
    std::string filename = "test.glb";

    bool ret = false;

    if (filename.ends_with(".glb")) {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    }
    else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    }

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

    std::cout << "Success! Loaded: " << filename << std::endl;

    std::cout << "Meshes:   " << model.meshes.size() << std::endl;
    std::cout << "Nodes:    " << model.nodes.size() << std::endl;
    std::cout << "Buffers:  " << model.buffers.size() << std::endl;
    std::cout << "Skins:    " << model.skins.size() << std::endl;
    std::cout << "Anims:    " << model.animations.size() << std::endl;
}