#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "Model.h"

class ModelManager
{
public:
    static ModelManager& GetInstance()
    {
        static ModelManager instance;
        return instance;
    }

    // Get or load a model (with automatic fallback resolution)
    std::shared_ptr<CModel> GetModel(const std::string& filePath);

    // パス解決 (Debug: GLB優先, Release: 暗号化.dat優先)
    std::string ResolveModelPath(const std::string& inputPath) const;

    void Clear() { m_modelCache.clear(); }

private:
    ModelManager() = default;
    ~ModelManager() = default;
    ModelManager(const ModelManager&) = delete;
    ModelManager& operator=(const ModelManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<CModel>> m_modelCache;
};
