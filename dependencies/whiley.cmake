include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Declare(
    whiley
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/whiley
    GIT_REPOSITORY https://github.com/dannybpoulsen/whiley.git
)

FetchContent_MakeAvailable(whiley)
