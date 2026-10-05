#pragma once

#include "ModelData.h"
#include <cstdint>
#include <string>

class IModelLoader
{
public:
    virtual ~IModelLoader() = default;

    virtual bool LoadFromMemory(const uint8_t* data, size_t size, const std::string& assetPathOrName, LoadedModelData& outData) = 0;
};
