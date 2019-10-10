add_library (linuxsupport OBJECT feedback.cpp)
target_include_directories (linuxsupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_compile_options(linuxsupport PUBLIC "-fPIC")
