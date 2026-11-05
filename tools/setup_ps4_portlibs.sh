#!/usr/bin/env bash
set -euo pipefail

: "${OO_PS4_TOOLCHAIN:?OO_PS4_TOOLCHAIN must be set}"

echo "==> Setting up PS4 portlibs (zlib, libpng, polarssl, curl)..."
WORKDIR="$(mktemp -d)"
trap 'rm -rf "$WORKDIR"' EXIT

echo "==> Cloning oosdk_libraries for source files..."
git clone --depth 1 https://github.com/bucanero/oosdk_libraries.git "$WORKDIR/oosdk_libraries"

PS4_CFLAGS="--target=x86_64-pc-freebsd12-elf -fPIC -O2 -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include -D__ORBIS__=1 -DPS4=1 -D__FreeBSD__=12 -D_DEFAULT_SOURCE -D_BSD_SOURCE"

AR_CMD="$(command -v llvm-ar 2>/dev/null || command -v llvm-ar-18 2>/dev/null || command -v ar)"
echo "==> Using archiver: $AR_CMD"

# 1. Build zlib
echo "==> Compiling zlib for PS4..."
(
    cd "$WORKDIR/oosdk_libraries/zlib_partial"
    # shellcheck disable=SC2086
    clang $PS4_CFLAGS -c -Iinclude source/*.c
    "$AR_CMD" rcs libz.a ./*.o
    cp -f include/zlib.h include/zconf.h "${OO_PS4_TOOLCHAIN}/include/"
    cp -f libz.a "${OO_PS4_TOOLCHAIN}/lib/"
)

# 2. Build libpng
echo "==> Compiling libpng for PS4..."
(
    cd "$WORKDIR/oosdk_libraries/libpng"
    # shellcheck disable=SC2086
    clang $PS4_CFLAGS -c -Iinclude source/*.c
    "$AR_CMD" rcs libpng.a ./*.o
    cp -f include/*.h "${OO_PS4_TOOLCHAIN}/include/"
    cp -f libpng.a "${OO_PS4_TOOLCHAIN}/lib/"
)

# 3. Build polarssl (SSL/TLS for libcurl)
echo "==> Compiling polarssl for PS4..."
(
    cd "$WORKDIR/oosdk_libraries/polarssl-1.3.9"
    # shellcheck disable=SC2086
    clang $PS4_CFLAGS -c -Iinclude library/*.c
    "$AR_CMD" rcs libpolarssl.a ./*.o
    cp -r include/polarssl "${OO_PS4_TOOLCHAIN}/include/"
    cp -f libpolarssl.a "${OO_PS4_TOOLCHAIN}/lib/"
)

# 4. Build libcurl (HTTP-only, static with PolarSSL)
echo "==> Compiling curl for PS4..."
(
    repo_dir="$(cd "$(dirname "$0")/.." && pwd)"
    mkdir -p "$WORKDIR/curl-build"
    cmake -S "$WORKDIR/oosdk_libraries/curl-7.64.1" -B "$WORKDIR/curl-build" \
      -DCMAKE_TOOLCHAIN_FILE="$repo_dir/tools/ps4.toolchain.cmake" \
      -DCMAKE_BUILD_TYPE=Release \
      -DBUILD_TESTING=OFF \
      -DBUILD_CURL_EXE=OFF \
      -DCURL_STATICLIB=ON \
      -DHTTP_ONLY=ON \
      -DCURL_DISABLE_LDAP=ON \
      -DCURL_DISABLE_LDAPS=ON \
      -DENABLE_IPV6=OFF \
      -DCURL_DISABLE_CRYPTO_AUTH=ON \
      -DENABLE_THREADED_RESOLVER=OFF \
      -DUSE_UNIX_SOCKETS=OFF \
      -DCMAKE_USE_POLARSSL=ON \
      -DCURL_CA_PATH=none \
      -DENABLE_MANUAL=OFF
    cmake --build "$WORKDIR/curl-build" --target libcurl -j"$(nproc)"
    mkdir -p "${OO_PS4_TOOLCHAIN}/include/curl"
    cp -f "$WORKDIR/oosdk_libraries/curl-7.64.1/include/curl/"*.h "${OO_PS4_TOOLCHAIN}/include/curl/"
    cp -f "$WORKDIR/curl-build/lib/libcurl.a" "${OO_PS4_TOOLCHAIN}/lib/"
)

echo "==> PS4 portlibs successfully built and installed to ${OO_PS4_TOOLCHAIN}!"
