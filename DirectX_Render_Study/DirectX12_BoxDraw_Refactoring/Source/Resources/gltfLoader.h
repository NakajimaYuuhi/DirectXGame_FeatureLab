#pragma once

#include "ModelData.h"
#include "GltfModelLoader.h"
#include <string>

class CgltfLoader
{};

LoadedModelData LoadModelDataFromFile(const std::string& filePath);
LoadedModelData TestLoadGLTF(std::string _fileName = "Assets/Model/OffensiveIdle.glb");
