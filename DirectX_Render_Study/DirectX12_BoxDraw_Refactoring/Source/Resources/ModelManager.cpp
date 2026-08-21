#include "ModelManager.h"

std::shared_ptr<CModel> ModelManager::GetModel(const std::string& filePath)
{
    // If the model is already loaded, return the cached instance.
    auto it = m_modelCache.find(filePath);
    if (it != m_modelCache.end())
    {
        return it->second;
    }

    // Otherwise, create a new model and load it.
    auto model = std::make_shared<CModel>();
    model->ModelLoad(filePath);
    m_modelCache[filePath] = model;

    return model;
}
