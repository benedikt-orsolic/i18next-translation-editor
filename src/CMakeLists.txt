cmake_minimum_required(VERSION 3.27.1)
project(i18n_translation_manager LANGUAGES C)

add_executable(${PROJECT_NAME} main.c)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK4 REQUIRED IMPORTED_TARGET gtk4)

target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::GTK4)
