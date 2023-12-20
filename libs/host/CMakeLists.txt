project (host CXX)
add_subdirectory (src)
add_subdirectory (tests)

install (TARGETS host FILE_SET HEADERS)
