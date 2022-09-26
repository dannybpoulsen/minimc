add_library (interpreter STATIC task.cpp lexer.cpp parser.cpp statemap.cpp ../../include/interpreter/statemap.hpp)
target_include_directories (interpreter PUBLIC ${PROJECT_SOURCE_DIR}/include )