import os
import shutil
import re
import glob

shader_dir = os.path.join('Assets', 'Shader')

# 1. フォルダの作成
if not os.path.exists(shader_dir):
    os.makedirs(shader_dir)

# 2. *.hlsl の移動
shader_files = glob.glob('*.hlsl') + glob.glob('*.hlsli')
for file in shader_files:
    if os.path.exists(file):
        shutil.move(file, os.path.join(shader_dir, file))

# 3. .vcxproj および .filters の更新
def update_project_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    for file in shader_files:
        # <FxCompile Include="Triangle.hlsl"> などのパスを Assets\Shader\Triangle.hlsl に変更
        content = re.sub(
            rf'(<(?:FxCompile|None|CustomBuild) Include=")({file})(".*?>)',
            rf'\1Assets\\Shader\\\2\3',
            content
        )
    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

update_project_file('DirectX12_BoxDraw_Refactoring.vcxproj')
update_project_file('DirectX12_BoxDraw_Refactoring.vcxproj.filters')

# 4. ソースコード内のパス置換
def replace_in_file(filepath):
    # ファイルのエンコーディングが Shift-JIS になっている
    with open(filepath, 'r', encoding='shift_jis', errors='ignore') as f:
        content = f.read()
    
    # L"Triangle.hlsl" -> L"Assets/Shader/Triangle.hlsl" などに置換
    for file in shader_files:
        content = re.sub(
            rf'L"{file}"',
            rf'L"Assets/Shader/{file}"',
            content
        )

    with open(filepath, 'w', encoding='shift_jis', errors='replace') as f:
        f.write(content)

cpp_files_to_update = [
    os.path.join('Source', 'Objects', 'Box.cpp'),
    os.path.join('Source', 'Rendering', 'PostProcessPass.cpp'),
    os.path.join('Source', 'Rendering', 'PSOManager.cpp'),
    os.path.join('Source', 'Resources', 'Material.h')
]

for f in cpp_files_to_update:
    if os.path.exists(f):
        replace_in_file(f)

print("Moved shaders successfully.")
