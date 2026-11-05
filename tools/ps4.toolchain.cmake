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
find_program(PS4_AR NAMES llvm-ar llvm-ar-18 ar)
if(PS4_AR)
    set(CMAKE_AR "${PS4_AR}" CACHE FILEPATH "Archiver")
endif()
find_program(PS4_RANLIB NAMES llvm-ranlib llvm-ranlib-18 ranlib)
if(PS4_RANLIB)
    set(CMAKE_RANLIB "${PS4_RANLIB}" CACHE FILEPATH "Ranlib")
endif()

find_program(PS4_LINKER NAMES ld.lld ld.lld-18 ld)
if(NOT PS4_LINKER)
    set(PS4_LINKER "ld.lld")
endif()

set(TRIPLE "x86_64-pc-freebsd12-elf")

set(PS4_DEFS "-D__ORBIS__=1 -DPS4=1 -D__FreeBSD__=12 -D__FreeBSD_version=1200000 -D_POSIX_C_SOURCE=200809L -D_XOPEN_SOURCE=700 -D__BSD_VISIBLE=1 -D_DEFAULT_SOURCE -D_BSD_SOURCE -DHAVE_UNISTD_H=1")

set(CMAKE_C_FLAGS_INIT "--target=${TRIPLE} -fPIC -funwind-tables -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include ${PS4_DEFS}")
set(CMAKE_CXX_FLAGS_INIT "--target=${TRIPLE} -fPIC -funwind-tables -isysroot ${OO_PS4_TOOLCHAIN} -isystem ${OO_PS4_TOOLCHAIN}/include/c++/v1 -isystem ${OO_PS4_TOOLCHAIN}/include ${PS4_DEFS}")

set(_PS4_LINK "\"${PS4_LINKER}\" -m elf_x86_64 -pie --script \"${OO_PS4_TOOLCHAIN}/link.x\" --eh-frame-hdr --no-rosegment -z notext --allow-multiple-definition -o <TARGET> \"${OO_PS4_TOOLCHAIN}/lib/crt1.o\" <OBJECTS> <LINK_FLAGS> -L\"${OO_PS4_TOOLCHAIN}/lib\" --start-group <LINK_LIBRARIES> -lc -lkernel -lc++ -lunwind --end-group")
set(CMAKE_C_LINK_EXECUTABLE "${_PS4_LINK}")
set(CMAKE_CXX_LINK_EXECUTABLE "${_PS4_LINK}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
