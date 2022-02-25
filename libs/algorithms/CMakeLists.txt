add_library (algorithms INTERFACE)
target_link_libraries (algorithms INTERFACE support cpa)


target_include_directories (algorithms INTERFACE ${PROJECT_SOURCE_DIR}/include)
