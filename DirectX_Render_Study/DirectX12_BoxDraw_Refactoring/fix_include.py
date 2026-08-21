import os

vcxproj_path = 'c:/Users/yuuhi/Cpp_Project/Study/DirectXGame_FeatureLab/DirectX_Render_Study/DirectX12_BoxDraw_Refactoring/DirectX12_BoxDraw_Refactoring.vcxproj'

with open(vcxproj_path, 'r', encoding='utf-8') as f:
    content = f.read()

# Make sure we don't duplicate it
if '$(ProjectDir)Source\\External;' not in content:
    new_content = content.replace('$(ProjectDir)Source\\Core;', '$(ProjectDir)Source\\External;$(ProjectDir)Source\\Core;')
    with open(vcxproj_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    print('Updated vcxproj with Source\\External')
else:
    print('Already updated.')
