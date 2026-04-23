add_library (loaderinter STATIC loader.cpp)

target_link_libraries (loaderinter PUBLIC model)
target_include_directories(loaderinter PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include> )

add_library(loaderheaders INTERFACE)
target_sources(loaderheaders
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/loaders/loader.hpp
)

install(TARGETS loaderheaders FILE_SET HEADERS)
