

add_library (smt SHARED smt.cpp)

target_link_libraries (smt PUBLIC host support  smtlib)
target_include_directories (smt PUBLIC  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>  )
INSTALL (TARGETS smtlib)

target_sources(smt
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/smt/smt.hpp
)

