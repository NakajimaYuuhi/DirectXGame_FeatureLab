import os
import sys

def convert_file_to_sjis(file_path):
    if not os.path.exists(file_path):
        print(f"[SKIP] File not found: {file_path}")
        return
    
    try:
        with open(file_path, 'rb') as f:
            raw_bytes = f.read()
        
        if not raw_bytes:
            return

        # Check UTF-8 BOM (0xEF, 0xBB, 0xBF)
        has_bom = raw_bytes.startswith(b'\xef\xbb\xbf')
        content_bytes = raw_bytes[3:] if has_bom else raw_bytes

        # Try decoding as UTF-8
        try:
            text = content_bytes.decode('utf-8')
            # Normalize wave dash (U+301C -> U+FF5E for CP932 compatibility)
            text = text.replace('\u301c', '\uff5e')
            sjis_bytes = text.encode('cp932', errors='replace')
            
            with open(file_path, 'wb') as f:
                f.write(sjis_bytes)
            print(f"[CONVERTED] UTF-8 -> Shift_JIS: {file_path}")
        except UnicodeDecodeError:
            # If already Shift_JIS (or another encoding), strip BOM header if present
            if has_bom:
                with open(file_path, 'wb') as f:
                    f.write(content_bytes)
                print(f"[STRIPPED BOM] Shift_JIS kept: {file_path}")
            else:
                print(f"[ALREADY SJIS] {file_path}")

    except Exception as e:
        print(f"[ERROR] Failed to convert {file_path}: {e}")

def main():
    target = sys.argv[1] if len(sys.argv) > 1 else "."
    
    if os.path.isfile(target):
        convert_file_to_sjis(target)
    elif os.path.isdir(target):
        exts = ('.cpp', '.h', '.hpp', '.c', '.inl', '.hlsl', '.hlsli', '.fx')
        print(f"=== Scanning target directory: {target} ===")
        for root, dirs, files in os.walk(target):
            # Ignore VCS and build directories
            if any(p in root for p in ('.git', '.vs', 'build', '.agents', 'bin', 'obj')):
                continue
            for file in files:
                if file.lower().endswith(exts):
                    convert_file_to_sjis(os.path.join(root, file))
    else:
        print(f"[ERROR] Target path not found: {target}")

if __name__ == "__main__":
    main()
