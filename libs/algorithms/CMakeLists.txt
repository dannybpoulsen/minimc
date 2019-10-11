add_library (algorithms)
target_link_libraries (algorithms PUBLIC support cpa)
target_include_directories (algorithms PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_subdirectory(tests)
