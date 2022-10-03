add_library (minimcloader OBJECT loader.cpp parser.cpp lexer.cpp lexer.hpp)
target_link_libraries (minimcloader PUBLIC support model util)
target_include_directories(minimcloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
