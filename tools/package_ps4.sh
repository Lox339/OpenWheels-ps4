#!/usr/bin/env bash
# Packages OpenWheels into a PS4 Fake PKG (OpenWheels-ps4.pkg) using CyB1K / Orbis Publishing Tools.
#
# Usage: tools/package_ps4.sh [--build-dir <dir>] [--out <dir>] [--assets <dir>]
#                             [--ios <dir>] [--data <dir>] [--tools-dir <dir>]
set -euo pipefail

repo="$(cd "$(dirname "$0")/.." && pwd)"
build_dir="$repo/build-ps4-release"
out="$repo/dist"
assets="$repo/binary/HappyWheels_Android/HW_Android/assets"
ios="$repo/binary/HappyWheels_iOS/Payload/happywheels.app"
data=""
tools_dir="${CYB1K_TOOLS_DIR:-$repo/tools/ps4_fake_pkg_tools}"

while [ $# -gt 0 ]; do
    case "$1" in
        --build-dir) build_dir="$2"; shift ;;
        --out) out="$2"; shift ;;
        --assets) assets="$2"; shift ;;
        --ios) ios="$2"; shift ;;
        --data) data="$2"; shift ;;
        --tools-dir) tools_dir="$2"; shift ;;
        -h|--help)
            echo "Usage: tools/package_ps4.sh [--build-dir <dir>] [--out <dir>] [--assets <dir>] [--ios <dir>] [--data <dir>] [--tools-dir <dir>]"
            exit 0
            ;;
        *) echo "unknown option: $1" >&2; exit 2 ;;
    esac
    shift
done

bin=""
for cand in "$build_dir/bin/OpenWheels" "$build_dir/bin" "$build_dir"; do
    if [ -f "$cand/eboot.bin" ] || [ -f "$cand/OpenWheels" ] || [ -f "$cand/OpenWheels.elf" ]; then
        bin="$cand"
        break
    fi
done
[ -n "$bin" ] || bin="$build_dir/bin/OpenWheels"

target_elf=""
if [ -f "$bin/OpenWheels.elf" ]; then
    target_elf="$bin/OpenWheels.elf"
elif [ -f "$bin/OpenWheels" ]; then
    target_elf="$bin/OpenWheels"
fi

if [ ! -f "$bin/eboot.bin" ] && [ -n "$target_elf" ]; then
    echo "Creating eboot.bin from $target_elf..."
    if command -v create-eboot >/dev/null 2>&1; then
        create-eboot "$target_elf" "$bin/eboot.bin"
    elif [ -n "${OO_PS4_TOOLCHAIN:-}" ] && [ -x "$OO_PS4_TOOLCHAIN/bin/linux/create-eboot" ]; then
        "$OO_PS4_TOOLCHAIN/bin/linux/create-eboot" "$target_elf" "$bin/eboot.bin"
    elif [ -n "${OO_PS4_TOOLCHAIN:-}" ] && [ -x "$OO_PS4_TOOLCHAIN/bin/linux/create-fself" ]; then
        "$OO_PS4_TOOLCHAIN/bin/linux/create-fself" -in="$target_elf" -out="$bin/OpenWheels.oelf" --eboot "$bin/eboot.bin" --paid 0x3800000000000011
    else
        echo "Warning: create-eboot not found, using binary directly as eboot.bin"
        cp "$target_elf" "$bin/eboot.bin"
    fi
fi

[ -f "$bin/eboot.bin" ] || { echo "eboot.bin not found in $bin" >&2; exit 1; }

staging="$(mktemp -d -t openwheels-ps4-staging-XXXXXX)"
trap 'rm -rf "$staging"' EXIT

echo "==> Staging PS4 package in $staging"
cp "$bin/eboot.bin" "$staging/eboot.bin"

data="${data:-$bin}"
for t in soundlist.tsv gametext.tsv; do
    [ -f "$data/$t" ] && cp -L "$data/$t" "$staging/"
done

[ -d "$data/generated" ] && cp -rL "$data/generated" "$staging/generated"

if [ -d "$repo/res/levels/restored" ]; then
    mkdir -p "$staging/levels"
    cp -r "$repo/res/levels/restored" "$staging/levels/restored"
fi

if [ -d "$assets" ]; then
    cp -rL "$assets" "$staging/assets"
fi

if [ -d "$ios" ]; then
    mkdir -p "$staging/ios"
    find -L "$ios" -maxdepth 1 -type f \( -name '*.png' -o -name '*.plist' -o -name '*.strings' -o -name '*.ttf' \
        -o -name '*.otf' -o -name '*.fnt' -o -name '*.xml' -o -name '*.json' \) ! -name Info.plist ! -name 'embedded*' \
        -exec cp {} "$staging/ios/" \;
fi

cp "$repo/LICENSE" "$repo/NOTICE.md" "$staging/"

icon=""
if [ -f "$repo/res/ps4/icon0.png" ]; then
    icon="$repo/res/ps4/icon0.png"
elif [ -f "$repo/docs/screenshots/main-menu.png" ]; then
    icon="$repo/docs/screenshots/main-menu.png"
fi

mkdir -p "$out"
python3 "$repo/tools/release/make_ps4_pkg.py" \
    --app-dir "$staging" \
    --out "$out/OpenWheels-ps4.pkg" \
    --tools-dir "$tools_dir" \
    ${icon:+--icon "$icon"}

echo "==> PS4 packaging finished successfully: $out/OpenWheels-ps4.pkg"
