add_subdirectory (llvm)
add_subdirectory (xxhash)

add_subdirectory (doctest)
if (ENABLE_PYTHON)
add_subdirectory (pybind11)
endif()
