add_subdirectory (locvm)
add_library (pathformula OBJECT Query.cpp)

target_link_Libraries ( pathformula PUBLIC model util hash  vm smt pathvm)
target_include_directories (pathformula PUBLIC ${PROJECT_SOURCE_DIR}/include)

