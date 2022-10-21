add_library (modelanalysis STATIC  loop.cpp dominator.cpp )
target_include_directories (modelanalysis PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (modelanalysis support util host)

