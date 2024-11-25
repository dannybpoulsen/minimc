find_package (FLEX REQUIRED)


flex_target(flexer lexer.yy "${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc")



add_library (minimcloader SHARED loader.cpp "${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc" parser.cpp )
target_link_libraries (minimcloader PUBLIC loaderinter model)
target_include_directories (minimcloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

INSTALL (TARGETS minimcloader)
add_library(MiniMC::minimcloader ALIAS minimcloader)
