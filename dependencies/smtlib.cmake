include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Declare(
    smtlib
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/smtlib
    GIT_REPOSITORY https://gitlab.com/dannybpoulsen/smtlib.git
    GIT_TAG  3e1648469b3b56e92a173c4e0a5cd1958c546080
)

FetchContent_MakeAvailable(smtlib)
