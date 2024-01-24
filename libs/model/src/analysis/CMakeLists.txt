add_library (modelanalysis OBJECT  loop.cpp dominator.cpp )
target_include_directories (modelanalysis PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries (modelanalysis modelisa  support util host)

