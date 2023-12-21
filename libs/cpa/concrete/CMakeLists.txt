project (cpa_concrete)
add_subdirectory (locvm)
add_library (concrete STATIC Query.cpp)

target_link_Libraries (concrete PUBLIC model util hash support vm vm_concrete cpa_internal)
target_include_directories (concrete PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(concrete_headers INTERFACE)
target_sources(concrete_headers
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/cpa/concrete.hpp
)

install(TARGETS concrete_headers EXPORT MiniMCTargets FILE_SET HEADERS)


