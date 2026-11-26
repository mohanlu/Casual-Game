cmake_minimum_required(VERSION 3.25)
project(ZombieGame VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_TOOLCHAIN_FILE)
    message(STATUS "Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include(config/cmake/Dependencies.cmake)

add_library(zombie_runtime
    src/windows.cpp
    src/input.cpp
    src/resources.cpp
    src/graphics.cpp
    src/entity.cpp
    src/item.cpp
    src/game_flow.cpp
    src/menu.cpp
    src/settings.cpp
)
target_link_libraries(zombie_runtime PUBLIC zombie_dependencies)
target_include_directories(zombie_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
add_executable(ZombieGame src/main.cpp)
target_link_libraries(ZombieGame PRIVATE zombie_runtime)
add_custom_target(copy_assets ALL
    COMMAND "${CMAKE_COMMAND}"
        "-DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/src/images"
        "-DDESTINATION=$<TARGET_FILE_DIR:ZombieGame>/assets/images"
        "-DBINARY_ROOT=${CMAKE_BINARY_DIR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/config/cmake/CopyAssets.cmake"
    COMMENT "Copy images, fonts, licenses and font sources"
)
add_dependencies(ZombieGame copy_assets)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(zombie_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    add_executable(runtime_tests tests/runtime_tests.cpp)
    target_link_libraries(runtime_tests PRIVATE zombie_runtime)
    add_dependencies(runtime_tests copy_assets)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-working-directory")
    add_test(NAME runtime_foundation COMMAND runtime_tests)
    set_tests_properties(runtime_foundation PROPERTIES TIMEOUT 15
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/test-working-directory"
        ENVIRONMENT "SDL_VIDEO_DRIVER=dummy;SDL_RENDER_DRIVER=software")
endif()
