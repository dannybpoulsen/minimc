project (storage CXX)

add_library (storage INTERFACE)
target_include_directories (storage INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)

INSTALL (TARGETS storage EXPORT MiniMCTargets FILE_SET HEADERS)
