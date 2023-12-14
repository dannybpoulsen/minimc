project (vm CXX)

add_library (vm INTERFACE)
target_include_directories (vm INTERFACE ${PROJECT_SOURCE_DIR}/include)

add_subdirectory (tests)
