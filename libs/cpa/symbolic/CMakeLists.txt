project (cpa_symb)
add_library (symb STATIC Query.cpp)

target_link_Libraries (symb PUBLIC model util hash support vm symb_vm cpa_internal)
target_include_directories (symb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories (symb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)

add_library(symb_headers INTERFACE)
target_sources(symb_headers
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
)

install(TARGETS symb_headers EXPORT MiniMCTargets FILE_SET HEADERS)


