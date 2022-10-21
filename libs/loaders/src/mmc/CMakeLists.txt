add_library (minimcloader OBJECT loader.cpp lexer.cpp parser.cpp tokens.hpp)
target_link_libraries (minimcloader PUBLIC model)
target_include_directories(minimcloader PUBLIC ${PROJECT_SOURCE_DIR}/include)
