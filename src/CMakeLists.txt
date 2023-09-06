wrap_resource(INPUT shaders/camera_frame.vs PREFIX shaders/camera_frame_vs NAMESPACE rcs TARGET camera_vs_code_gen SOURCE_VAR rcs_source_files)
wrap_resource(INPUT shaders/camera_frame_yuv420.fs PREFIX shaders/camera_frame_yuv420_fs NAMESPACE rcs TARGET camera_fs_yuv420_code_gen SOURCE_VAR rcs_source_files)
wrap_resource(INPUT shaders/camera_frame_rgb.fs PREFIX shaders/camera_frame_rgb_fs NAMESPACE rcs TARGET camera_fs_rgb_code_gen SOURCE_VAR rcs_source_files)
wrap_resource(INPUT shaders/camera_frame_dma.fs PREFIX shaders/camera_frame_dma_fs NAMESPACE rcs TARGET camera_fs_dma_code_gen SOURCE_VAR rcs_source_files)

file(GLOB_RECURSE glhelper_source_files glhelpers/*.h glhelpers/*.cpp)
add_library(glhelpers STATIC ${glhelper_source_files})
target_link_libraries(glhelpers SDL2-static OpenGL::OpenGL PkgConfig::epoxy)
target_include_directories(glhelpers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(glhelpers PUBLIC cxx_std_17)
target_compile_definitions(glhelpers PUBLIC -DGL_GLEXT_PROTOTYPES=1)

file(GLOB elem_source_files elements/*.h elements/*.cpp)
add_library(naive_parts STATIC
    ${rcs_source_files}
    ${elem_source_files}
    CameraReader.h CameraReader.cpp
    LightSensorThread.h LightSensorThread.cpp
    RenderThreadBase.h RenderThreadBase.cpp
    RenderThread_YUV420.h RenderThread_YUV420.cpp
    RenderThread_RGB888.h RenderThread_RGB888.cpp
    RenderThread_EglDma.h RenderThread_EglDma.cpp
    SdlHelpers.h
    TaskQueue.h
    Utils.h Utils.cpp
    LuxSetterThread.h LuxSetterThread.cpp
    Settings.h Settings.cpp)
target_link_libraries(naive_parts
    glhelpers
    PkgConfig::libcamera
    PkgConfig::libdrm
    PkgConfig::libgpiod
    SDL2-static
    OpenGL::OpenGL
    OpenGL::EGL
    atomic
    pthread)
target_include_directories(naive_parts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
add_dependencies(naive_parts camera_vs_code_gen)

add_executable(naive_view naive_view.cpp)
target_link_libraries(naive_view naive_parts)

install(TARGETS naive_view DESTINATION ${CMAKE_INSTALL_BINDIR})