#include "GamePackageBuilder.h"
#include "BuildRunner.h"
#include "Source/Resources/AssetEncryptor.h"
#include <Windows.h>
#include <shellapi.h>
#include <filesystem>

namespace fs = std::filesystem;

bool GamePackageBuilder::StartPackageBuild(const PackageBuildConfig& config)
{
    BuildRunner& runner = BuildRunner::GetInstance();
    if (runner.IsRunning())
    {
        return false;
    }

    PackageBuildConfig capturedConfig = config;

    return runner.StartMSBuildAsync(
        "DirectX12_BoxDraw_Refactoring.sln",
        config.configuration,
        "x64",
        "/m /v:minimal",
        [this, capturedConfig](bool buildSuccess, int exitCode)
        {
            BuildRunner& br = BuildRunner::GetInstance();
            if (!buildSuccess)
            {
                br.AppendLog("[PackageBuilder] MSBuild failed with exit code " + std::to_string(exitCode) + ". Packaging aborted.");
                br.SetState(BuildState::Failed);
                return;
            }

            br.AppendLog("");
            br.AppendLog("==========================================");
            br.AppendLog("[PackageBuilder] MSBuild succeeded! Starting asset packaging...");
            br.AppendLog("==========================================");

            if (PackageAssetsAndBinaries(capturedConfig))
            {
                br.AppendLog("==========================================");
                br.AppendLog("[PackageBuilder] Package build completed successfully!");
                br.AppendLog("  Output Directory: " + fs::absolute(capturedConfig.outputDir).string());
                br.AppendLog("  Executable      : " + capturedConfig.appName);
                br.AppendLog("==========================================");
                br.SetState(BuildState::Succeeded);
            }
            else
            {
                br.AppendLog("[PackageBuilder] Failed during asset/binary packaging step.");
                br.SetState(BuildState::Failed);
            }
        }
    );
}

bool GamePackageBuilder::PackageAssetsAndBinaries(const PackageBuildConfig& config)
{
    BuildRunner& br = BuildRunner::GetInstance();

    try
    {
        fs::path outRoot(config.outputDir);

        if (config.cleanOutputBeforeBuild && fs::exists(outRoot))
        {
            br.AppendLog("[PackageBuilder] Cleaning existing output directory: " + outRoot.string());
            std::error_code ec;
            fs::remove_all(outRoot, ec);
        }

        fs::create_directories(outRoot);

        // 1. Copy built executable & runtime DLLs / CSOs from x64/<Config>
        fs::path binDir = fs::path("x64") / config.configuration;
        fs::path srcExe = binDir / "DirectX12_BoxDraw_Refactoring.exe";

        if (!fs::exists(srcExe))
        {
            br.AppendLog("[Error] Built executable not found at: " + srcExe.string());
            return false;
        }

        std::string exeName = config.appName;
        if (!exeName.ends_with(".exe"))
        {
            exeName += ".exe";
        }

        fs::path dstExe = outRoot / exeName;
        fs::copy_file(srcExe, dstExe, fs::copy_options::overwrite_existing);
        br.AppendLog("[PackageBuilder] Copied executable -> " + dstExe.string());

        // Copy any .dll (e.g. GameLogic.dll) and .cso files from build output dir
        if (fs::exists(binDir))
        {
            for (const auto& entry : fs::directory_iterator(binDir))
            {
                if (!entry.is_regular_file()) continue;
                std::string ext = entry.path().extension().string();
                std::string fname = entry.path().filename().string();
                if (ext == ".dll" || ext == ".cso")
                {
                    // Skip temporary hot-reload DLL copies
                    if (fname.find("_HotReload_") != std::string::npos) continue;

                    fs::path dstFile = outRoot / entry.path().filename();
                    fs::copy_file(entry.path(), dstFile, fs::copy_options::overwrite_existing);
                    br.AppendLog("[PackageBuilder] Copied binary -> " + dstFile.string());
                }
            }
        }

        // 2. Copy & Cook Assets/ directory
        fs::path assetsSrc("Assets");
        fs::path assetsDst = outRoot / "Assets";

        if (fs::exists(assetsSrc))
        {
            int copiedCount = 0;
            int encryptedCount = 0;

            for (const auto& entry : fs::recursive_directory_iterator(assetsSrc))
            {
                fs::path relPath = fs::relative(entry.path(), assetsSrc);
                fs::path targetPath = assetsDst / relPath;

                if (entry.is_directory())
                {
                    fs::create_directories(targetPath);
                    continue;
                }

                if (!entry.is_regular_file()) continue;

                fs::create_directories(targetPath.parent_path());
                std::string ext = entry.path().extension().string();

                // .glb モデルの暗号化処理
                if (ext == ".glb" && config.encryptModels)
                {
                    fs::path datTarget = targetPath;
                    datTarget.replace_extension(".dat");

                    if (AssetSecurity::EncryptModelFile(entry.path().string(), datTarget.string()))
                    {
                        encryptedCount++;
                        br.AppendLog("[PackageBuilder] Encrypted model: " + relPath.string() + " -> " + datTarget.filename().string());
                    }
                    else
                    {
                        br.AppendLog("[Warning] Failed to encrypt model: " + relPath.string());
                    }

                    // 平文 .glb も残す設定の場合のみコピー
                    if (!config.excludePlaintextModels)
                    {
                        fs::copy_file(entry.path(), targetPath, fs::copy_options::overwrite_existing);
                        copiedCount++;
                    }
                    continue;
                }

                // 既存の .dat は、同名 .glb から今生成した場合は上書きしない
                if (ext == ".dat" && config.encryptModels)
                {
                    fs::path correspondingGlb = entry.path();
                    correspondingGlb.replace_extension(".glb");
                    if (fs::exists(correspondingGlb))
                    {
                        continue;
                    }
                }

                fs::copy_file(entry.path(), targetPath, fs::copy_options::overwrite_existing);
                copiedCount++;
            }

            br.AppendLog("[PackageBuilder] Assets packaged: " + std::to_string(copiedCount) + " files copied, " +
                         std::to_string(encryptedCount) + " models encrypted (.dat).");
        }

        return true;
    }
    catch (const std::exception& e)
    {
        br.AppendLog(std::string("[Exception] PackageAssetsAndBinaries: ") + e.what());
        return false;
    }
}

void GamePackageBuilder::OpenOutputFolder(const std::string& outputDir) const
{
    fs::path absPath = fs::absolute(outputDir);
    if (!fs::exists(absPath))
    {
        fs::create_directories(absPath);
    }
    ShellExecuteA(NULL, "open", absPath.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
}

bool GamePackageBuilder::LaunchBuiltGame(const PackageBuildConfig& config) const
{
    std::string exeName = config.appName;
    if (!exeName.ends_with(".exe"))
    {
        exeName += ".exe";
    }

    fs::path absRoot = fs::absolute(config.outputDir);
    fs::path absExe = absRoot / exeName;
    if (!fs::exists(absExe))
    {
        return false;
    }

    STARTUPINFOA si = {};
    si.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION pi = {};

    std::string cmd = "\"" + absExe.string() + "\"";
    std::vector<char> cmdBuf(cmd.begin(), cmd.end());
    cmdBuf.push_back('\0');

    BOOL ok = CreateProcessA(
        NULL,
        cmdBuf.data(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        absRoot.string().c_str(),
        &si,
        &pi
    );

    if (ok)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return true;
    }
    return false;
}
