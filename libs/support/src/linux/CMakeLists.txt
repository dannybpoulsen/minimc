add_library (linuxsupport OBJECT feedback.cpp)
target_link_libraries(linuxsupport PUBLIC io)
target_include_directories (linuxsupport PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_options(linuxsupport PUBLIC "-fPIC")
