add_executable (succ succegenerator.cpp)
target_link_libraries  (succ model support algorithms loader)

add_executable (pgraph pgraph.cpp)
target_link_libraries  (pgraph model support algorithms loader)
