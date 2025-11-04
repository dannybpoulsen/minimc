project (vm CXX)

add_library (vm INTERFACE)

target_include_directories (vm INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_link_libraries(vm INTERFACE smt)
add_subdirectory (tests)

install (TARGETS vm  EXPORT MiniMCTargets FILE_SET HEADERS
)
