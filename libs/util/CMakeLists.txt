project(util CXX)

add_library (util INTERFACE)
target_include_directories (util INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include> )
target_link_libraries (util INTERFACE hash )

target_sources(util
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/util/valuemap.hpp
)


install (TARGETS  util  EXPORT MiniMCTargets FILE_SET HEADERS)
