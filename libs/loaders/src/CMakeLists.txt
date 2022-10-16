add_library (loader SHARED loader.cpp)
add_subdirectory (llvm)
target_link_libraries (loader PUBLIC llvmloader)
target_include_directories(loader PUBLIC ${PROJECT_SOURCE_DIR}/include)

INSTALL (TARGETS loader)
