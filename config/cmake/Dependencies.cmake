find_package(SDL3 CONFIG REQUIRED)
find_package(SDL3_image CONFIG REQUIRED)
find_package(SDL3_ttf CONFIG REQUIRED)
find_package(EnTT 3.12.0 EXACT CONFIG REQUIRED)

if(TARGET SDL3_image::SDL3_image-shared)
    set(ZOMBIE_IMAGE_TARGET SDL3_image::SDL3_image-shared)
else()
    set(ZOMBIE_IMAGE_TARGET SDL3_image::SDL3_image-static)
endif()

add_library(zombie_dependencies INTERFACE)
target_link_libraries(zombie_dependencies INTERFACE
    SDL3::SDL3
    ${ZOMBIE_IMAGE_TARGET}
    SDL3_ttf::SDL3_ttf
    EnTT::EnTT
)
target_compile_features(zombie_dependencies INTERFACE cxx_std_17)

# CMake generates the temporary compile/link probe in the build directory.
# No game entry point is needed during environment setup.
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_LIBRARIES SDL3::SDL3 ${ZOMBIE_IMAGE_TARGET} SDL3_ttf::SDL3_ttf EnTT::EnTT)
check_cxx_source_compiles("
    #include <SDL3/SDL.h>
    #include <SDL3_image/SDL_image.h>
    #include <SDL3_ttf/SDL_ttf.h>
    #include <entt/entity/registry.hpp>
    int main() {
        entt::registry registry;
        const auto entity = registry.create();
        return SDL_GetVersion() + IMG_Version() + TTF_Version() + registry.valid(entity);
    }
" ZOMBIE_DEPENDENCIES_LINK)
unset(CMAKE_REQUIRED_LIBRARIES)
if(NOT ZOMBIE_DEPENDENCIES_LINK)
    message(FATAL_ERROR "C++17 dependency compile/link check failed. See CMakeFiles/CMakeConfigureLog.yaml.")
endif()

