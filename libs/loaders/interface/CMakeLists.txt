add_library (loaderinter STATIC loader.cpp)

target_link_libraries (loaderinter PUBLIC model)
target_include_directories(loaderinter PUBLIC ${PROJECT_SOURCE_DIR}/include)

add_library(loaderheaders INTERFACE)
target_sources(loaderheaders
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/loaders/loader.hpp
)

install(TARGETS loaderheaders FILE_SET HEADERS)
