cmake_minimum_required(VERSION 3.14)
project(NaiveView)

include(cmake/WrapResource.cmake)
include(GNUInstallDirs)

find_package(Perl REQUIRED)
find_package(PkgConfig REQUIRED)
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED COMPONENTS EGL OpenGL)
pkg_check_modules(libcamera REQUIRED IMPORTED_TARGET libcamera)
pkg_check_modules(libdrm REQUIRED IMPORTED_TARGET libdrm)
pkg_check_modules(epoxy REQUIRED IMPORTED_TARGET epoxy)
pkg_check_modules(libgpiod REQUIRED IMPORTED_TARGET libgpiod)

add_subdirectory(SDL)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(playground)

install(FILES naive_view.service DESTINATION /${CMAKE_INSTALL_SYSCONFDIR}/systemd/system)