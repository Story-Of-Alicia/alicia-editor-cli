cmake_minimum_required(VERSION 3.31)
project(alicia-editor-cli)

add_subdirectory(3rd-party)

add_library(project_properties
        INTERFACE)
target_compile_features(project_properties
        INTERFACE cxx_std_20)

if (MSVC)
    target_compile_options(project_properties INTERFACE /W4 /WX)
else()
    target_compile_options(project_properties INTERFACE -Wall -Wextra -pedantic -Werror)
endif()

add_library(libpak
    src/libpak/libpak.cpp)
target_include_directories(libpak
        PUBLIC include/)
target_link_libraries(libpak
        PUBLIC zlibstatic project_properties)


add_executable(alicia-editor-cli
        src/editor/main.cpp)
target_link_libraries(alicia-editor-cli
        PRIVATE libpak project_properties)
