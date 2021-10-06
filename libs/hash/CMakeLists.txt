find_package (XXHash)

add_library (hash STATIC)
target_sources (hash PRIVATE hash.cpp)
target_include_directories(hash PUBLIC ${PROJECT_SOURCE_DIR}/include)
message ("USe ${XXHash_LIBRARIES}")
target_link_libraries (hash PUBLIC ${XXHash_LIBRARIES})
target_compile_options (hash PRIVATE -Wall -Wpedantic -Wextra)
