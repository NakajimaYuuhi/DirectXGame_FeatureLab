#pragma once

#include "IModelLoader.h"
#include <memory>

class GltfModelLoader : public IModelLoader
{
public:
    GltfModelLoader() = default;
    ~GltfModelLoader() override = default;

    bool LoadFromMemory(const uint8_t* data, size_t size, const std::string& assetPathOrName, LoadedModelData& outData) override;
};
