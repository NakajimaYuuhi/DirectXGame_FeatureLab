#include "ModelManager.h"
#include <filesystem>
#include <iostream>

std::string ModelManager::ResolveModelPath(const std::string& inputPath) const
{
    std::string base = inputPath;
    std::string ext = "";
    size_t dotPos = inputPath.find_last_of('.');
    size_t slashPos = inputPath.find_last_of("/\\");
    if (dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos))
    {
        base = inputPath.substr(0, dotPos);
        ext = inputPath.substr(dotPos);
    }

#ifdef NDEBUG
    // リリースビルド: 暗号化ファイル優先
    std::string datPath = base + ".dat";
    if (std::filesystem::exists(datPath))
    {
        return datPath;
    }
    if (std::filesystem::exists(inputPath))
    {
        return inputPath;
    }
#else
    // デバッグ・エディタビルド: 生のGLBがあれば開発優先、なければ暗号化.dat
    std::string glbPath = base + ".glb";
    if (std::filesystem::exists(glbPath))
    {
        return glbPath;
    }
    std::string datPath = base + ".dat";
    if (std::filesystem::exists(datPath))
    {
        return datPath;
    }
    if (std::filesystem::exists(inputPath))
    {
        return inputPath;
    }
#endif

    return inputPath;
}

std::shared_ptr<CModel> ModelManager::GetModel(const std::string& filePath)
{
    std::string resolvedPath = ResolveModelPath(filePath);

    // キャッシュ確認 (解決前または解決後のキー)
    auto it = m_modelCache.find(resolvedPath);
    if (it != m_modelCache.end())
    {
        return it->second;
    }
    auto itRaw = m_modelCache.find(filePath);
    if (itRaw != m_modelCache.end())
    {
        return itRaw->second;
    }

    auto model = std::make_shared<CModel>();
    model->ModelLoad(resolvedPath);
    model->SetModelPath(filePath);

    m_modelCache[resolvedPath] = model;
    m_modelCache[filePath] = model;

    return model;
}
