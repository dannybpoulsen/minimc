add_library (interpreter OBJECT task.cpp lexer.cpp parser.cpp statemap.cpp miniinterpreter.cpp)
target_link_libraries (interpreter PUBLIC cpa algorithms loader)
target_include_directories (interpreter PUBLIC ${PROJECT_SOURCE_DIR})
