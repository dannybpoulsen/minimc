add_library (hash STATIC hash.cpp)
target_include_directories(hash PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (hash PUBLIC xxhash_imp)
target_compile_options (hash PRIVATE -Wall -Wpedantic -Wextra)
