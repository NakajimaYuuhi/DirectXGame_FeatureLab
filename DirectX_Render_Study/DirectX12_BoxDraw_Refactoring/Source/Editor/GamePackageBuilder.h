#pragma once

#include <string>

struct PackageBuildConfig
{
    std::string outputDir = "Build/ReleasePackage";
    std::string appName = "DirectX12_Game.exe";
    std::string configuration = "Release";
    bool encryptModels = true;
    bool excludePlaintextModels = true;
    bool cleanOutputBeforeBuild = true;
};

class GamePackageBuilder
{
public:
    static GamePackageBuilder& GetInstance()
    {
        static GamePackageBuilder instance;
        return instance;
    }

    // Start full build & asset packaging asynchronously
    bool StartPackageBuild(const PackageBuildConfig& config);

    // Package already-built binaries and assets synchronously (called by worker thread after MSBuild succeeds)
    bool PackageAssetsAndBinaries(const PackageBuildConfig& config);

    // Open output folder in Windows Explorer
    void OpenOutputFolder(const std::string& outputDir) const;

    // Launch the built executable
    bool LaunchBuiltGame(const PackageBuildConfig& config) const;

private:
    GamePackageBuilder() = default;
    ~GamePackageBuilder() = default;
    GamePackageBuilder(const GamePackageBuilder&) = delete;
    GamePackageBuilder& operator=(const GamePackageBuilder&) = delete;
};
