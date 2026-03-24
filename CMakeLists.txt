cmake_minimum_required(VERSION 3.27)
project(alicia-editor-cli)

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
