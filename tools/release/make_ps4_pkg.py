#!/usr/bin/env python3
"""
PS4 Fake PKG Builder for OpenWheels.
Uses CyB1K / Orbis Publishing Tools (orbis-pub-cmd.exe via Wine on Linux).
Generates param.sfo, project.gp4, and creates OpenWheels-ps4.pkg.
"""

import argparse
import os
import shutil
import struct
import subprocess
import sys
import xml.etree.ElementTree as ET


def build_param_sfo(entries):
    """Generates a binary Sony Param.SFO file from key-value pairs."""
    sorted_entries = sorted(entries.items(), key=lambda x: x[0])
    num_entries = len(sorted_entries)

    key_table = bytearray()
    data_table = bytearray()
    index_table = bytearray()

    key_offsets = {}
    for key, _ in sorted_entries:
        key_offsets[key] = len(key_table)
        key_table.extend(key.encode("utf-8") + b"\x00")

    data_offsets = {}
    for key, val in sorted_entries:
        while len(data_table) % 4 != 0:
            data_table.append(0)
        data_offsets[key] = len(data_table)

        if isinstance(val, int):
            fmt = 0x0404  # uint32 integer
            raw = struct.pack("<I", val)
            max_len = 4
            data_len = 4
        elif isinstance(val, str):
            fmt = 0x0204  # null-terminated string
            raw = val.encode("utf-8") + b"\x00"
            max_len = (len(raw) + 7) & ~7
            data_len = len(raw)
            raw = raw.ljust(max_len, b"\x00")
        else:
            raise TypeError(f"Unsupported SFO type: {type(val)}")

        index_table.extend(struct.pack("<HHIII", key_offsets[key], fmt, data_len, max_len, data_offsets[key]))
        data_table.extend(raw)

    while len(key_table) % 4 != 0:
        key_table.append(0)

    key_table_offset = 20 + len(index_table)
    data_table_offset = key_table_offset + len(key_table)

    header = struct.pack("<4s4sIII", b"\x00PSF", b"\x01\x01\x00\x00", key_table_offset, data_table_offset, num_entries)
    return header + index_table + key_table + data_table


def create_blank_png(path, width=512, height=512):
    """Creates a minimal valid 512x512 RGBA PNG file if none exists."""
    import zlib
    raw_data = bytearray()
    # Gradient blue/orange icon
    for y in range(height):
        raw_data.append(0)  # filter type 0 (None)
        for x in range(width):
            r = int(220 * (x / width))
            g = int(120 * (y / height))
            b = 40
            a = 255
            raw_data.extend([r, g, b, a])

    compressed = zlib.compress(raw_data)

    def png_chunk(chunk_type, data):
        return struct.pack(">I", len(data)) + chunk_type + data + struct.pack(">I", zlib.crc32(chunk_type + data) & 0xFFFFFFFF)

    png = b"\x89PNG\r\n\x1a\n"
    ihdr_data = struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)
    png += png_chunk(b"IHDR", ihdr_data)
    png += png_chunk(b"IDAT", compressed)
    png += png_chunk(b"IEND", b"")

    with open(path, "wb") as f:
        f.write(png)


def generate_gp4(app_dir, output_gp4, content_id, passcode="00000000000000000000000000000000"):
    """Generates an Orbis Pub Gen / Fake PKG Tools GP4 project file."""
    root = ET.Element("pssproject", fmt="gp4")
    volume = ET.SubElement(root, "volume")
    ET.SubElement(volume, "volume_type").text = "pkg_ps4_app"
    ET.SubElement(volume, "volume_id").text = "PS4VOLUME"
    ET.SubElement(volume, "volume_ts").text = "2026-11-05 00:00:00"

    package = ET.SubElement(volume, "package", paramsfo="sce_sys/param.sfo")
    ET.SubElement(package, "passcode").text = passcode
    ET.SubElement(package, "content_id").text = content_id

    files_elem = ET.SubElement(root, "files", default_split_size="0")
    rootdir_elem = ET.SubElement(root, "rootdir")

    # Collect directories and files
    app_dir = os.path.abspath(app_dir)
    all_dirs = set()
    all_files = []

    for dirpath, dirnames, filenames in os.walk(app_dir):
        rel_dir = os.path.relpath(dirpath, app_dir).replace("\\", "/")
        if rel_dir != ".":
            all_dirs.add(rel_dir)
        for fname in filenames:
            full_path = os.path.join(dirpath, fname)
            rel_file = os.path.relpath(full_path, app_dir).replace("\\", "/")
            all_files.append((full_path, rel_file))

    # Add directory hierarchy
    for d in sorted(all_dirs):
        parts = d.split("/")
        if len(parts) == 1:
            ET.SubElement(rootdir_elem, "dir", targ_name=parts[0])

    # Add files
    for full_path, rel_path in sorted(all_files, key=lambda x: x[1]):
        ET.SubElement(files_elem, "file", targ_path=rel_path).text = full_path

    tree = ET.ElementTree(root)
    ET.indent(tree, space="  ")
    tree.write(output_gp4, encoding="utf-8", xml_declaration=True)
    print(f"Generated GP4: {output_gp4} with {len(all_files)} files")


def main():
    parser = argparse.ArgumentParser(description="Package OpenWheels into a PS4 Fake PKG (.pkg)")
    parser.add_argument("--app-dir", required=True, help="Directory containing PS4 app payload (eboot.bin, assets, etc.)")
    parser.add_argument("--out", required=True, help="Output .pkg file destination")
    parser.add_argument("--tools-dir", default=None, help="Path to CyB1K PS4-Fake-PKG-Tools directory")
    parser.add_argument("--title-id", default="CUSA33900", help="Title ID (e.g. CUSA33900)")
    parser.add_argument("--content-id", default="IV0000-CUSA33900_00-OPENWHEELS000000", help="Content ID")
    parser.add_argument("--passcode", default="00000000000000000000000000000000", help="PKG Passcode (32 zeros)")
    parser.add_argument("--icon", default=None, help="Custom 512x512 icon0.png")
    args = parser.parse_args()

    app_dir = os.path.abspath(args.app_dir)
    sce_sys = os.path.join(app_dir, "sce_sys")
    os.makedirs(sce_sys, exist_ok=True)

    # 1. Create param.sfo
    sfo_path = os.path.join(sce_sys, "param.sfo")
    sfo_data = build_param_sfo({
        "APP_TYPE": 1,
        "APP_VER": "01.00",
        "CATEGORY": "gd",
        "CONTENT_ID": args.content_id,
        "DOWNLOAD_DATA_SIZE": 0,
        "SYSTEM_VER": 0x05050000,
        "TITLE": "OpenWheels",
        "TITLE_ID": args.title_id,
        "VERSION": "01.00",
    })
    with open(sfo_path, "wb") as f:
        f.write(sfo_data)
    print(f"Created: {sfo_path}")

    # 2. Create icon0.png
    icon_dst = os.path.join(sce_sys, "icon0.png")
    if args.icon and os.path.isfile(args.icon):
        shutil.copyfile(args.icon, icon_dst)
        print(f"Copied icon from {args.icon}")
    elif not os.path.isfile(icon_dst):
        create_blank_png(icon_dst)
        print(f"Created default 512x512 icon: {icon_dst}")

    # 3. Generate project.gp4
    gp4_path = os.path.join(app_dir, "project.gp4")
    generate_gp4(app_dir, gp4_path, args.content_id, args.passcode)

    # 4. Locate orbis-pub-cmd
    pub_cmd = None
    if args.tools_dir:
        cand = os.path.join(args.tools_dir, "orbis-pub-cmd.exe")
        if os.path.isfile(cand):
            pub_cmd = cand

    if not pub_cmd:
        # Search PATH
        pub_cmd = shutil.which("orbis-pub-cmd.exe") or shutil.which("orbis-pub-cmd")

    out_pkg = os.path.abspath(args.out)
    os.makedirs(os.path.dirname(out_pkg), exist_ok=True)

    if pub_cmd:
        print(f"Using publishing tool: {pub_cmd}")
        cmd = []
        if sys.platform != "win32" and pub_cmd.endswith(".exe"):
            cmd = ["wine", pub_cmd, "img_create", "--oformat", "pkg", gp4_path, out_pkg]
        else:
            cmd = [pub_cmd, "img_create", "--oformat", "pkg", gp4_path, out_pkg]

        print("Executing:", " ".join(cmd))
        subprocess.check_call(cmd)
        print(f"Successfully built PS4 PKG: {out_pkg}")
    else:
        print("Warning: orbis-pub-cmd executable not found. Generated GP4 and app structure are ready in:", app_dir)


if __name__ == "__main__":
    main()
