add_library (io SHARED os_ostream.cpp)
target_include_directories (io PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include> )
