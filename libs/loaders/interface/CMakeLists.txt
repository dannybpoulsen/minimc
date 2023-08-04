add_library (loaderinter STATIC loader.cpp)

target_link_libraries (loaderinter PUBLIC model)
target_include_directories(loaderinter PUBLIC ${PROJECT_SOURCE_DIR}/include)
