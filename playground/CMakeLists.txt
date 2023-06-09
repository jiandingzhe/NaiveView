
add_executable(frame-fetch frame-fetch.cpp)
target_link_libraries(frame-fetch PkgConfig::libcamera)
target_compile_features(frame-fetch PUBLIC cxx_std_17)

add_executable(show_video_drivers show_video_drivers.cpp)
target_link_libraries(show_video_drivers SDL2-static)
target_compile_features(show_video_drivers PUBLIC cxx_std_17)

add_executable(bh1750_play bh1750_play.cpp)
target_compile_features(bh1750_play PUBLIC cxx_std_17)

add_executable(draw_something_kms draw_something_kms.cpp)
target_link_libraries(draw_something_kms glhelpers)

add_executable(set_screen_brightness set_screen_brightness.cpp)

add_executable(set_gpio set_gpio.cpp)
target_link_libraries(set_gpio PkgConfig::libgpiod)

add_executable(dual_bh1750_play dual_bh1750_play.cpp)
target_link_libraries(dual_bh1750_play naive_parts)
