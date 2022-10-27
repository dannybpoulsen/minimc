add_library (vm_path STATIC)
target_sources (vm_path PRIVATE operations.cpp casts.cpp pathformula.cpp)
target_include_directories (vm_path PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (vm_path PRIVATE smtlib PUBLIC model smt)
