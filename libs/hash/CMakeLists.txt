find_package (XXHash)

add_library (hash STATIC)
target_sources (hash PRIVATE hash.cpp)
target_include_directories(hash PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
message ("USe ${XXHASH_LIBRARIES}")
target_link_libraries (hash PUBLIC ${XXHASH_LIBRARIES})
target_compile_options (hash PRIVATE -Wall -Wpedantic -Wextra)
