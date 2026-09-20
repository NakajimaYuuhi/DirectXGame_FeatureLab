#pragma once
#include <string>

class CObject;

class PrefabSerializer
{
public:
    static bool SavePrefab(const std::string& filepath, CObject* obj);
};
