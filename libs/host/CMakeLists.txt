project (host CXX)
add_subdirectory (src)
add_subdirectory (tests)

add_library(MiniMC::host ALIAS host)
