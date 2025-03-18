add_library (io SHARED os_ostream.cpp prompter.cpp)
target_include_directories (io PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include> )
target_link_libraries (io PUBLIC readline)
