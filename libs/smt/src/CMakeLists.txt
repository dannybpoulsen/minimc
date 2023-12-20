

add_library (smt SHARED smt.cpp)

target_link_libraries (smt PUBLIC smtlib host support)
target_include_directories (smt PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include  )
INSTALL (TARGETS smtlib)

target_sources(smt
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/smt/smt.hpp
)
