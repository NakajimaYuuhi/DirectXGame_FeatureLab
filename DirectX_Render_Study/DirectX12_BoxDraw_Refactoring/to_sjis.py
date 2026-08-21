import os

files_to_convert = [
    'Shader.h', 'Shader.cpp',
    'ShaderManager.h', 'ShaderManager.cpp',
    'PSOBuilder.h', 'PSOBuilder.cpp',
    'PSOManager.h', 'PSOManager.cpp',
    'Material.h', 'Material.cpp'
]

for file in files_to_convert:
    if os.path.exists(file):
        # UTF-8で読み込んでみる
        try:
            with open(file, 'r', encoding='utf-8') as f:
                content = f.read()
            # Shift-JISで書き直す
            with open(file, 'w', encoding='shift_jis', errors='replace') as f:
                f.write(content)
            print(f"Converted {file} to Shift-JIS")
        except UnicodeDecodeError:
            # すでにShift-JISなどの場合はスキップ
            print(f"Skipped {file} (likely already Shift-JIS)")
