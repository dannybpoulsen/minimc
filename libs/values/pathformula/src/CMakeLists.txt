add_library (pathvm STATIC)
target_sources (pathvm PRIVATE operations.cpp casts.cpp pathformula.cpp)
target_include_directories (pathvm PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (pathvm PRIVATE smtlib PUBLIC model smt vm cpa_headers cpa_internal)
