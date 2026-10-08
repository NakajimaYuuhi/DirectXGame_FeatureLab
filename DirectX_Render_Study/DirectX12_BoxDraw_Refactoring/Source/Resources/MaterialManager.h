#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <filesystem>
#include "Material.h"

class MaterialManager
{
public:
    static MaterialManager& GetInstance()
    {
        static MaterialManager instance;
        return instance;
    }

    std::shared_ptr<CMaterial> GetMaterial(const std::string& filePath)
    {
        auto it = m_materials.find(filePath);
        if (it != m_materials.end())
        {
            return it->second;
        }

        auto mat = std::make_shared<CMaterial>();
        if (mat->LoadFromFile(filePath))
        {
            m_materials[filePath] = mat;
            return mat;
        }

        return nullptr;
    }

    std::shared_ptr<CMaterial> CreateInstance(const std::string& filePath)
    {
        auto base = GetMaterial(filePath);
        if (base)
        {
            return base->Clone();
        }
        return nullptr;
    }

    std::vector<std::string> GetAvailableMaterialFiles() const
    {
        std::vector<std::string> files;
        try
        {
            const std::string dir = "Assets/Materials";
            if (std::filesystem::exists(dir))
            {
                for (const auto& entry : std::filesystem::directory_iterator(dir))
                {
                    if (entry.is_regular_file() && entry.path().extension() == ".mat")
                    {
                        files.push_back(entry.path().generic_string());
                    }
                }
            }
        }
        catch (...) {}
        return files;
    }

    void ReloadMaterial(const std::string& filePath)
    {
        auto it = m_materials.find(filePath);
        if (it != m_materials.end() && it->second)
        {
            it->second->LoadFromFile(filePath);
        }
    }

    void Clear()
    {
        m_materials.clear();
    }

private:
    MaterialManager() = default;
    ~MaterialManager() = default;
    MaterialManager(const MaterialManager&) = delete;
    MaterialManager& operator=(const MaterialManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<CMaterial>> m_materials;
};
