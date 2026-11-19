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

add_executable(ZombieGame
    src/main.cpp
    src/windows.cpp
)
target_link_libraries(ZombieGame PRIVATE zombie_dependencies)
target_include_directories(ZombieGame PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ZombieGame PRIVATE -Wall -Wextra -Wpedantic)
endif()
