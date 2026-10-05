#pragma once

#include <string>

namespace AssetSecurity
{
    // モデルファイルをXOR暗号化してヘッダー付きで出力
    bool EncryptModelFile(const std::string& inputPath, const std::string& outputPath);

    // 暗号化されたモデルファイルを復号して平文ファイルとして出力
    bool DecryptModelFile(const std::string& inputPath, const std::string& outputPath);
}
