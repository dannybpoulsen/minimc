add_library (algorithms STATIC)
target_link_libraries (algorithms PUBLIC support cpa)
target_precompile_headers (algorithms PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/algorithms/enumstates.hpp)

target_include_directories (algorithms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
