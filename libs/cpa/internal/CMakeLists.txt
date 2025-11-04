project(cpa_internal)
add_library(cpa_internal INTERFACE)

target_include_directories (cpa_internal INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(cpa_internal INTERFACE smt vm)
