# CMake toolchain file for PlayStation 4 using OpenOrbis PS4 Toolchain
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(PS4 TRUE)

if(DEFINED ENV{OO_PS4_TOOLCHAIN})
    set(OO_PS4_TOOLCHAIN "$ENV{OO_PS4_TOOLCHAIN}")
elseif(NOT DEFINED OO_PS4_TOOLCHAIN)
    set(OO_PS4_TOOLCHAIN "/opt/OpenOrbis/PS4Toolchain")
endif()

set(CMAKE_SYSROOT "${OO_PS4_TOOLCHAIN}")
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_LINKER ld.lld)

set(TRIPLE "x86_64-pc-freebsd12-elf")

set(CMAKE_C_FLAGS_INIT "--target=${TRIPLE} -fPIC -funwind-tables -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include -D__ORBIS__=1 -DPS4=1")
set(CMAKE_CXX_FLAGS_INIT "${CMAKE_C_FLAGS_INIT} -isystem ${OO_PS4_TOOLCHAIN}/include/c++/v1")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -Wl,-m,elf_x86_64 -Wl,-pie -Wl,--script=${OO_PS4_TOOLCHAIN}/link.x -Wl,--eh-frame-hdr -L${OO_PS4_TOOLCHAIN}/lib ${OO_PS4_TOOLCHAIN}/lib/crt1.o")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
