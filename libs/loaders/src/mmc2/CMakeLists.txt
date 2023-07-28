find_package (FLEX REQUIRED)


flex_target(flexer lexer.yy "${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc")

#add_executable (flextest "${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc")
#target_link_libraries (flextest PUBLIC model)
#target_include_directories(flextest  PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})


add_library (minimcloader OBJECT loader.cpp "${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.cc" parser.cpp )
target_link_libraries (minimcloader PUBLIC model)
target_include_directories(minimcloader PUBLIC ${PROJECT_SOURCE_DIR}/include PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
