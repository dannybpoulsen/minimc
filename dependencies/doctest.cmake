include(FetchContent)
find_package(Git REQUIRED)
FetchContent_Declare(
    doctest
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/doctest
    GIT_REPOSITORY https://github.com/doctest/doctest.git
    GIT_TAG 2.4.7
)

if(NOT doctest_POPULATED)
    FetchContent_Populate(doctest)
    add_subdirectory(${doctest_SOURCE_DIR} ${doctest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()


FetchContent_MakeAvailable (doctest)
