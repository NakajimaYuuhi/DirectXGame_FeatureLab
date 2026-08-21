import re

cpp_files = ['Shader.cpp', 'ShaderManager.cpp', 'PSOBuilder.cpp']
h_files = ['Shader.h', 'ShaderManager.h', 'PSOBuilder.h']

def update_vcxproj(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        text = f.read()

    # cppの追加
    for cpp in cpp_files:
        if f'Include="{cpp}"' not in text:
            text = re.sub(r'(<ClCompile Include="main\.cpp" />)', rf'\1\n    <ClCompile Include="{cpp}" />', text)

    # hの追加
    for h in h_files:
        if f'Include="{h}"' not in text:
            text = re.sub(r'(<ClInclude Include="main\.h" />)', rf'\1\n    <ClInclude Include="{h}" />', text)
            # main.hがない場合のフォールバックとして DX12Manager.h を探す
            if f'Include="{h}"' not in text:
                text = re.sub(r'(<ClInclude Include="DX12Manager\.h" />)', rf'\1\n    <ClInclude Include="{h}" />', text)

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(text)

def update_filters(filename):
    with open(filename, 'r', encoding='utf-8') as f:
        text = f.read()

    # cppの追加
    for cpp in cpp_files:
        if f'Include="{cpp}"' not in text:
            block = f'''<ClCompile Include="{cpp}">
      <Filter>ソース ファイル</Filter>
    </ClCompile>'''
            text = re.sub(r'(<ClCompile Include="main\.cpp">.*?<\/ClCompile>)', rf'\1\n    {block}', text, flags=re.DOTALL)

    # hの追加
    for h in h_files:
        if f'Include="{h}"' not in text:
            block = f'''<ClInclude Include="{h}">
      <Filter>ヘッダー ファイル</Filter>
    </ClInclude>'''
            text = re.sub(r'(<ClInclude Include="DX12Manager\.h">.*?<\/ClInclude>)', rf'\1\n    {block}', text, flags=re.DOTALL)

    with open(filename, 'w', encoding='utf-8') as f:
        f.write(text)

update_vcxproj('DirectX12_BoxDraw_Refactoring.vcxproj')
update_filters('DirectX12_BoxDraw_Refactoring.vcxproj.filters')
