import os
import shutil
import re

# 1. フォルダとファイルの分類定義
folders = {
    'Core': [
        'DX12Manager.h', 'DX12Manager.cpp',
        'InputManager.h', 'InputManager.cpp',
        'SceneManager.h', 'SceneManager.cpp',
        'Scene.h', 'Scene.cpp',
        'SceneTest.h', 'SceneTest.cpp',
        'SceneTitle.h', 'SceneTitle.cpp',
        'Manager.h', 'Manager.cpp',
        'main.cpp'
    ],
    'Rendering': [
        'ForwardRenderPass.h', 'ForwardRenderPass.cpp',
        'PostProcessPass.h', 'PostProcessPass.cpp',
        'RenderPipeline.h', 'RenderPipeline.cpp',
        'PSOManager.h', 'PSOManager.cpp',
        'PSOBuilder.h', 'PSOBuilder.cpp',
        'ShaderManager.h', 'ShaderManager.cpp',
        'Shader.h', 'Shader.cpp',
        'TextureManager.h', 'TextureManager.cpp',
        'DescriptorHeapAllocator.h', 'DescriptorHeapAllocator.cpp',
        'RenderTexture.h', 'RenderTexture.cpp'
    ],
    'Resources': [
        'Texture.h', 'Texture.cpp',
        'Material.h', 'Material.cpp',
        'Mesh.h', 'Mesh.cpp',
        'Model.h', 'Model.cpp',
        'ModelManager.h', 'ModelManager.cpp',
        'gltfLoader.h', 'gltfLoader.cpp',
        'ModelData.h',
        'Bone.h', 'Bone.cpp'
    ],
    'Components': [
        'Transform.h', 'Transform.cpp',
        'Collider.h', 'Collider.cpp',
        'Collider3D.h', 'Collider3D.cpp',
        'BoxCollider3D.h', 'BoxCollider3D.cpp',
        'Collision.h', 'Collision.cpp',
        'Component.h', 'Component.cpp'
    ],
    'Objects': [
        'Object.h', 'Object.cpp',
        'ObjectManager.h', 'ObjectManager.cpp',
        'ObjectManager_Getter_Setter.cpp',
        'ObjectManager_Instantiate.cpp',
        '3D_Object.h', '3D_Object.cpp',
        'Player.h', 'Player.cpp',
        'Enemy.h', 'Enemy.cpp',
        'Bullet.h', 'Bullet.cpp',
        'BillBoard.h', 'BillBoard.cpp',
        'Skydome.h', 'Skydome.cpp',
        'RandomParticle.h', 'RandomParticle.cpp',
        'Explosion.h', 'Explosion.cpp',
        'EventManager.h', 'EventManager.cpp',
        'Event.h', 'Event.cpp',
        'EventData.h', 'EventData.cpp',
        'EventData_NextScene.h', 'EventData_NextScene.cpp',
        'ButtonEventManager.h', 'ButtonEventManager.cpp',
        'EnemyCount.h', 'EnemyCount.cpp',
        'EnemyCounter.h', 'EnemyCounter.cpp',
        'ObjectInfo.h', 'ObjectInfo.cpp'
    ],
    'UI': [
        'ImGuiManager.h', 'ImGuiManager.cpp',
        'SpriteRenderer.h', 'SpriteRenderer.cpp',
        'TextRenderer.h', 'TextRenderer.cpp',
        'TitleUI.h', 'TitleUI.cpp',
        'CUIButton.h', 'CUIButton.cpp',
        'TextObject.h', 'TextObject.cpp',
        'UIObject.h', 'UIObject.cpp'
    ],
    'Util': [
        'BasicSettings.h',
        'StringAlias.h',
        'SmartPtrAlias.h',
        'D2DTextRenderer.h', 'D2DTextRenderer.cpp',
        'audio.h', 'audio.cpp'
    ]
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

# 追加のインクルードディレクトリを設定
# <AdditionalIncludeDirectories> を <ClCompile> 内部に追加する
# 既存のものがあれば置き換え、なければ追加する
include_dirs = ";".join([f"$(ProjectDir){f}" for f in folders.keys()])
include_tag = f"<AdditionalIncludeDirectories>%(AdditionalIncludeDirectories);{include_dirs}</AdditionalIncludeDirectories>"

# 全ての <ClCompile> ブロック内（ItemDefinitionGroup）に注入
vcxproj_content = re.sub(
    r'(<ItemDefinitionGroup.*?>\s*<ClCompile>\s*)',
    rf'\1{include_tag}\n      ',
    vcxproj_content
)

# ファイルのパスを更新
for folder, files in folders.items():
    for file in files:
        # <ClCompile Include="main.cpp" /> -> <ClCompile Include="Core\main.cpp" />
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
        # Includeパスの更新と、子要素の <Filter> を書き換える
        pattern = rf'<(ClCompile|ClInclude) Include="{file}">\s*<Filter>.*?</Filter>\s*</\1>'
        replacement = rf'<\g<1> Include="{folder}\\{file}">\n      <Filter>{folder}</Filter>\n    </\g<1>>'
        
        # まずは置き換え対象があるか確認して置換
        if re.search(pattern, filters_content):
            filters_content = re.sub(pattern, replacement, filters_content)
        else:
            # もし <ClCompile Include="hoge.cpp" /> と一行で書かれていたら
            pattern_single = rf'<(ClCompile|ClInclude) Include="{file}" />'
            replacement_single = rf'<\g<1> Include="{folder}\\{file}">\n      <Filter>{folder}</Filter>\n    </\g<1>>'
            filters_content = re.sub(pattern_single, replacement_single, filters_content)

# Filter自体の定義を追加
filters_def = ""
for folder in folders.keys():
    filters_def += f'''    <Filter Include="{folder}">
      <UniqueIdentifier>{{{os.urandom(16).hex()}}}</UniqueIdentifier>
    </Filter>\n'''

filters_content = re.sub(
    r'(<ItemGroup>\s*<Filter Include="ソース ファイル">)',
    rf'<ItemGroup>\n{filters_def}    <Filter Include="ソース ファイル">',
    filters_content
)

with open(filters_file, 'w', encoding='utf-8') as f:
    f.write(filters_content)

print("Project reorganization complete.")
