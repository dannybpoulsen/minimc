project (cpa_pathformula)


add_subdirectory (locvm)
add_library (pathformula STATIC Query.cpp)

target_link_Libraries ( pathformula PUBLIC model util hash  vm smt pathvm cpa_headers PRIVATE cpa_internal)
target_include_directories (pathformula PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(pathformula_headers INTERFACE)
target_sources(pathformula_headers
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/cpa/pathformula.hpp
)

install(TARGETS pathformula_headers FILE_SET HEADERS)
