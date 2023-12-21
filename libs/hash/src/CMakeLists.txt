add_library (hash SHARED hash.cpp)
target_include_directories(hash PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:/include>)
target_link_libraries (hash PRIVATE xxhash_imp)
target_compile_options (hash PRIVATE -Wall -Wpedantic -Wextra)

target_sources(hash
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/hash/hashing.hpp
)
