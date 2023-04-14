cmake_minimum_required(VERSION 3.14)
project(NaiveView)

find_package(PkgConfig REQUIRED)
find_package(SDL2 REQUIRED)
message("SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
message("SDL2_INCLUDE_DIRS: ${SDL2_INCLUDE_DIRS}")
pkg_check_modules(libcamera REQUIRED IMPORTED_TARGET libcamera)

add_subdirectory(playground)
