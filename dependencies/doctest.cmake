include(FetchContent)
find_package(Git REQUIRED)
FetchContent_Declare(
    doctest
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG v2.5.3
)




FetchContent_MakeAvailable (doctest)
