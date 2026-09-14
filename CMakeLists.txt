cmake_minimum_required(VERSION 3.27)
project(alicia-editor-cli)

# Statically link the MSVC C++ runtime (/MT) so the resulting .exe runs on a
# clean Windows machine without the Visual C++ Redistributable installed.
# Applies to all targets below, including the libpak / zlib subprojects.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(3rd-party)

add_library(project_properties
        INTERFACE)
target_compile_features(project_properties
        INTERFACE cxx_std_23)

if (MSVC)
    target_compile_options(project_properties INTERFACE /W4 /WX)
else()
    target_compile_options(project_properties INTERFACE -Wall -Wextra -pedantic -Werror)
endif()

add_executable(alicia-editor-cli
        src/editor/main.cpp)
target_link_libraries(alicia-editor-cli
        PRIVATE project_properties libpak)
