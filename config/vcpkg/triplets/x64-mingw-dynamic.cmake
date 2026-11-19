# Match vcpkg's community MinGW dynamic triplet.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_ENV_PASSTHROUGH_UNTRACKED PATH)
set(VCPKG_CMAKE_SYSTEM_NAME MinGW)
set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)

# GCC 16.2 crashes in ch_vect while compiling SDL_blit_auto.c at -O3.
# Keep the workaround scoped to SDL3's Release C compilation.
if(PORT STREQUAL "sdl3")
    set(VCPKG_C_FLAGS_RELEASE "-fno-tree-vectorize")
    set(VCPKG_CXX_FLAGS_RELEASE "")
endif()
