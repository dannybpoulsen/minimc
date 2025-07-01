include(FetchContent)
find_package(Git REQUIRED)

FetchContent_Declare(
    smtlib
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/smtlib
    GIT_REPOSITORY https://gitlab.com/dannybpoulsen/smtlib.git
    GIT_TAG  afafd038c675911915ab0893218df46440ad7848
)

FetchContent_MakeAvailable(smtlib)
