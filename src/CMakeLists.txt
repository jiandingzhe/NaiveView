wrap_resource(INPUT shaders/camera_frame.vs PREFIX shaders/camera_frame_vs NAMESPACE rcs TARGET camera_vs_code_gen SOURCE_VAR rcs_source_files)
wrap_resource(INPUT shaders/camera_frame.fs PREFIX shaders/camera_frame_fs NAMESPACE rcs TARGET camera_fs_code_gen SOURCE_VAR rcs_source_files)
file(GLOB_RECURSE glhelper_source_files glhelpers/*.h glhelpers/*.cpp)
add_library(glhelpers STATIC ${glhelper_source_files})
target_link_libraries(glhelpers SDL2::SDL2 OpenGL::OpenGL)
target_include_directories(glhelpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(glhelpers PUBLIC cxx_std_17)

add_library(naive_parts STATIC
    ${rcs_source_files}
    CameraReader.h CameraReader.cpp
    LightSensorThread.h LightSensorThread.cpp
    RenderThread.h RenderThread.cpp
    SdlHelpers.h
    TaskQueue.h)
target_link_libraries(naive_parts glhelpers PkgConfig::libcamera SDL2::SDL2 OpenGL::OpenGL atomic pthread)
target_include_directories(naive_parts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(naive_parts camera_vs_code_gen)

add_executable(naive_view naive_view.cpp)
target_link_libraries(naive_view naive_parts)