add_executable(queue_test queue_test.cpp)
target_include_directories(queue_test PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(queue_test atomic pthread)
target_compile_features(queue_test PUBLIC cxx_std_17)