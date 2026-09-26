import os
import sys

# 適用対象の拡張子
EXTENSIONS = ('.cpp', '.h', '.hpp', '.c', '.cs', '.inl', '.hlsl', '.hlsli', '.fx', '.md')

def convert_file(file_path):
    if not os.path.isfile(file_path):
        print(f"[SKIP] ファイルが存在しません: {file_path}")
        return

    if not file_path.lower().endswith(EXTENSIONS):
        return

    try:
        with open(file_path, 'rb') as f:
            raw_bytes = f.read()

        if not raw_bytes:
            return

        # BOMチェック (UTF-8 BOM: 0xEF, 0xBB, 0xBF)
        has_bom = raw_bytes.startswith(b'\xef\xbb\xbf')
        content_bytes = raw_bytes[3:] if has_bom else raw_bytes

        # すでに Shift_JIS (CP932) として正しくデコードできるか判定
        is_already_sjis = False
        try:
            content_bytes.decode('cp932')
            is_already_sjis = True
        except UnicodeDecodeError:
            is_already_sjis = False

        # UTF-8 としてデコードできるか判定
        is_utf8 = False
        utf8_text = ""
        try:
            utf8_text = content_bytes.decode('utf-8')
            is_utf8 = True
        except UnicodeDecodeError:
            is_utf8 = False

        # UTF-8 であり、かつ BOM 付与または Shift_JIS で未エンコードの場合に変換実行
        if is_utf8 and (has_bom or not is_already_sjis):
            # 波線 U+301C を U+FF5E (CP932互換) へ正規化
            utf8_text = utf8_text.replace('\u301c', '\uff5e')
            sjis_bytes = utf8_text.encode('cp932', errors='replace')
            with open(file_path, 'wb') as f:
                f.write(sjis_bytes)
            print(f"[CONVERTED UTF-8 -> Shift_JIS] {file_path}")
        elif has_bom and is_already_sjis:
            # Shift_JIS だが UTF-8 BOM が付いている場合は BOM だけ除去
            with open(file_path, 'wb') as f:
                f.write(content_bytes)
            print(f"[STRIPPED BOM] {file_path}")
        else:
            print(f"[OK Shift_JIS] {file_path}")

    except Exception as e:
        print(f"[ERROR] 処理失敗 {file_path}: {e}")

def process_target(target):
    if os.path.isfile(target):
        convert_file(target)
    elif os.path.isdir(target):
        print(f"=== フォルダ探索中: {target} ===")
        for root, dirs, files in os.walk(target):
            # ビルド生成物・VCSフォルダはスキップ
            if any(p in root for p in ('.git', '.vs', 'build', 'bin', 'obj', '.agents', 'DirectX1.503199cd', 'x64')):
                continue
            for file in files:
                convert_file(os.path.join(root, file))
    else:
        print(f"[WARN] 無効なパス指定: {target}")

def main():
    targets = sys.argv[1:]
    if not targets:
        # 引数がない場合は "Source" フォルダ（無ければカレントディレクトリ）を自動検索
        default_dir = "Source" if os.path.exists("Source") else "."
        process_target(default_dir)
    else:
        for t in targets:
            process_target(t)

if __name__ == "__main__":
    main()
