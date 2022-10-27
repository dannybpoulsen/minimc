include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Declare(
    smtlib
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/doctest
    GIT_REPOSITORY https://gitlab.com/dannybpoulsen/smtlib.git
)

FetchContent_MakeAvailable(smtlib)
