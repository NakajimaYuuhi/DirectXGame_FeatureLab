import os
import shutil
import re

# 1. 漏れていたファイルの追加分類
folders = {
    'Objects': ['Box.h', 'Box.cpp', 'Camera.h', 'Camera.cpp'],
    'Core': ['SceneEnums.h', 'EventEnums.h', 'ObjectTag.h'],
    'Rendering': ['IRenderPass.h', 'RenderContext.h'],
    'UI': ['ISelectable.h'],
    'Util': ['ContainerAlias.h', 'DirectXPtrAlias.h']
}

# 2. フォルダの作成とファイルの移動
for folder, files in folders.items():
    if not os.path.exists(folder):
        os.makedirs(folder)
    for file in files:
        if os.path.exists(file):
            shutil.move(file, os.path.join(folder, file))

# 3. .vcxproj の更新
vcxproj_file = 'DirectX12_BoxDraw_Refactoring.vcxproj'
with open(vcxproj_file, 'r', encoding='utf-8') as f:
    vcxproj_content = f.read()

# AdditionalIncludeDirectories に $(ProjectDir) を追加
vcxproj_content = re.sub(
    r'<AdditionalIncludeDirectories>%\(AdditionalIncludeDirectories\);(.*?)<\/AdditionalIncludeDirectories>',
    r'<AdditionalIncludeDirectories>%(AdditionalIncludeDirectories);$(ProjectDir);\1</AdditionalIncludeDirectories>',
    vcxproj_content
)

# ファイルのパスを更新
for folder, files in folders.items():
    for file in files:
        vcxproj_content = re.sub(
            rf'(<(?:ClCompile|ClInclude) Include=")({file})(".*?>)',
            rf'\1{folder}\\\2\3',
            vcxproj_content
        )

with open(vcxproj_file, 'w', encoding='utf-8') as f:
    f.write(vcxproj_content)

# 4. .vcxproj.filters の更新
filters_file = 'DirectX12_BoxDraw_Refactoring.vcxproj.filters'
with open(filters_file, 'r', encoding='utf-8') as f:
    filters_content = f.read()

# ファイルパスの更新とFilterの更新
for folder, files in folders.items():
    for file in files:
        pattern = rf'<(ClCompile|ClInclude) Include="{file}">\s*<Filter>.*?</Filter>\s*</\1>'
        replacement = rf'<\g<1> Include="{folder}\\{file}">\n      <Filter>{folder}</Filter>\n    </\g<1>>'
        
        if re.search(pattern, filters_content):
            filters_content = re.sub(pattern, replacement, filters_content)
        else:
            pattern_single = rf'<(ClCompile|ClInclude) Include="{file}" />'
            replacement_single = rf'<\g<1> Include="{folder}\\{file}">\n      <Filter>{folder}</Filter>\n    </\g<1>>'
            filters_content = re.sub(pattern_single, replacement_single, filters_content)

with open(filters_file, 'w', encoding='utf-8') as f:
    f.write(filters_content)

print("Fix reorganization complete.")
