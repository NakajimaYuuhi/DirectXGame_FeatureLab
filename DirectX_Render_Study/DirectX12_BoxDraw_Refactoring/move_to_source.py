import os
import shutil
import re

folders = ['Core', 'Rendering', 'Resources', 'Components', 'Objects', 'UI', 'Util']
source_dir = 'Source'

# 1. Sourceフォルダを作成し、既存のフォルダを移動
if not os.path.exists(source_dir):
    os.makedirs(source_dir)

for folder in folders:
    if os.path.exists(folder):
        shutil.move(folder, os.path.join(source_dir, folder))

# 2. .vcxproj の更新
vcxproj_file = 'DirectX12_BoxDraw_Refactoring.vcxproj'
with open(vcxproj_file, 'r', encoding='utf-8') as f:
    vcxproj_content = f.read()

# AdditionalIncludeDirectories のパスを更新
# <AdditionalIncludeDirectories>%(AdditionalIncludeDirectories);$(ProjectDir);$(ProjectDir)Core;...
for folder in folders:
    vcxproj_content = vcxproj_content.replace(f'$(ProjectDir){folder}', f'$(ProjectDir){source_dir}\\{folder}')

# ファイルのパスを更新
# <ClCompile Include="Core\main.cpp"> -> <ClCompile Include="Source\Core\main.cpp">
for folder in folders:
    vcxproj_content = re.sub(
        rf'(<(?:ClCompile|ClInclude) Include=")({folder}\\)',
        rf'\1{source_dir}\\\2',
        vcxproj_content
    )

with open(vcxproj_file, 'w', encoding='utf-8') as f:
    f.write(vcxproj_content)

# 3. .vcxproj.filters の更新
filters_file = 'DirectX12_BoxDraw_Refactoring.vcxproj.filters'
with open(filters_file, 'r', encoding='utf-8') as f:
    filters_content = f.read()

# ファイルパスの更新
for folder in folders:
    filters_content = re.sub(
        rf'(<(?:ClCompile|ClInclude) Include=")({folder}\\)',
        rf'\1{source_dir}\\\2',
        filters_content
    )

with open(filters_file, 'w', encoding='utf-8') as f:
    f.write(filters_content)

print("Moved to Source directory successfully.")
