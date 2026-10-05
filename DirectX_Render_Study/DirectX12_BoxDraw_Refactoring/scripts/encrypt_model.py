#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Model Encryption CLI Tool
Encrypts GLB / 3D model files into encrypted .dat files with an AssetFileHeader,
using XOR stream cipher with CRC32 integrity check.
"""

import sys
import os
import struct
import zlib
import argparse
from pathlib import Path

# Constants
ASSET_MAGIC_ENCRYPTED = 0x45594741  # 'AGYE'
VERSION = 1
CIPHER_XOR = 1
DEFAULT_XOR_KEY = b"XOR_MODEL_SECURE"
HEADER_FORMAT = "<IHHQQI8s"
HEADER_SIZE = struct.calcsize(HEADER_FORMAT)  # 36 bytes

def xor_crypt(data: bytes, key: bytes = DEFAULT_XOR_KEY) -> bytes:
    key_len = len(key)
    return bytes(b ^ key[i % key_len] for i, b in enumerate(data))

def encrypt_file(input_path: str, output_path: str, key: bytes = DEFAULT_XOR_KEY) -> bool:
    if not os.path.exists(input_path):
        print(f"[Error] Input file not found: {input_path}")
        return False

    with open(input_path, "rb") as f:
        raw_data = f.read()

    original_size = len(raw_data)
    checksum = zlib.crc32(raw_data) & 0xFFFFFFFF
    encrypted_data = xor_crypt(raw_data, key)
    encrypted_size = len(encrypted_data)

    header = struct.pack(
        HEADER_FORMAT,
        ASSET_MAGIC_ENCRYPTED,
        VERSION,
        CIPHER_XOR,
        original_size,
        encrypted_size,
        checksum,
        b"\x00" * 8
    )

    with open(output_path, "wb") as f:
        f.write(header)
        f.write(encrypted_data)

    print(f"[Encrypted] {input_path} -> {output_path} ({original_size} bytes -> {len(header) + encrypted_size} bytes)")
    return True

def decrypt_file(input_path: str, output_path: str, key: bytes = DEFAULT_XOR_KEY) -> bool:
    if not os.path.exists(input_path):
        print(f"[Error] Input file not found: {input_path}")
        return False

    with open(input_path, "rb") as f:
        header_bytes = f.read(HEADER_SIZE)
        if len(header_bytes) < HEADER_SIZE:
            print(f"[Error] File too small to contain header: {input_path}")
            return False

        magic, ver, cipher_type, orig_size, enc_size, checksum, reserved = struct.unpack(HEADER_FORMAT, header_bytes)
        if magic != ASSET_MAGIC_ENCRYPTED:
            print(f"[Error] Invalid magic: {magic:#x} (expected {ASSET_MAGIC_ENCRYPTED:#x})")
            return False

        if cipher_type != CIPHER_XOR:
            print(f"[Error] Unsupported cipher type: {cipher_type}")
            return False

        encrypted_data = f.read(enc_size)
        if len(encrypted_data) != enc_size:
            print(f"[Error] Incomplete encrypted data: expected {enc_size}, got {len(encrypted_data)}")
            return False

    decrypted_data = xor_crypt(encrypted_data, key)
    if len(decrypted_data) != orig_size:
        print(f"[Warning] Size mismatch: decrypted {len(decrypted_data)} != original {orig_size}")

    calc_crc = zlib.crc32(decrypted_data) & 0xFFFFFFFF
    if calc_crc != checksum:
        print(f"[Warning] Checksum mismatch: {calc_crc:#x} != {checksum:#x}")

    with open(output_path, "wb") as f:
        f.write(decrypted_data)

    print(f"[Decrypted] {input_path} -> {output_path} ({orig_size} bytes)")
    return True

def batch_encrypt(model_dir: str):
    p = Path(model_dir)
    count = 0
    for glb_file in p.glob("*.glb"):
        dat_file = glb_file.with_suffix(".dat")
        if encrypt_file(str(glb_file), str(dat_file)):
            count += 1
    print(f"[Batch Complete] Encrypted {count} models in {model_dir}")

def main():
    parser = argparse.ArgumentParser(description="Model Asset Encryptor / Decryptor CLI")
    parser.add_argument("input", nargs="?", help="Input file path")
    parser.add_argument("output", nargs="?", help="Output file path")
    parser.add_argument("--decrypt", action="store_true", help="Decrypt instead of encrypt")
    parser.add_argument("--batch", help="Batch encrypt all .glb files in specified directory")

    args = parser.parse_args()

    if args.batch:
        batch_encrypt(args.batch)
        return

    if not args.input:
        parser.print_help()
        return

    out_file = args.output
    if not out_file:
        if args.decrypt:
            out_file = str(Path(args.input).with_suffix(".glb"))
        else:
            out_file = str(Path(args.input).with_suffix(".dat"))

    if args.decrypt:
        decrypt_file(args.input, out_file)
    else:
        encrypt_file(args.input, out_file)

if __name__ == "__main__":
    main()
