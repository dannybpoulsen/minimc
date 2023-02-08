project (cpa CXX)

add_library (cpa_headers INTERFACE)
target_include_directories (cpa_headers INTERFACE ${PROJECT_SOURCE_DIR}/include)
add_subdirectory (src)
