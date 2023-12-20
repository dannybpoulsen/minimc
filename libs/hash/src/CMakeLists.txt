add_library (hash SHARED hash.cpp)
target_include_directories(hash PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (hash PUBLIC xxhash_imp)
target_compile_options (hash PRIVATE -Wall -Wpedantic -Wextra)

target_sources(hash
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/hash/hashing.hpp
)
