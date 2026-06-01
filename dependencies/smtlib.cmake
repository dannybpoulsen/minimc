include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Declare(
    smtlib
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/smtlib
    GIT_REPOSITORY https://gitlab.com/dannybpoulsen/smtlib.git
    GIT_TAG  origin/master
)

FetchContent_MakeAvailable(smtlib)
