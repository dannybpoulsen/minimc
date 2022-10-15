find_package(RapidJSON)

add_library (jsonsaver OBJECT saver.cpp)
if (RAPIDJSON_FOUND)
target_compile_definitions (jsonsaver PUBLIC RAPIDJSON)
endif (RAPIDJSON_FOUND)

target_link_libraries (jsonsaver  PUBLIC support util)
target_include_directories (jsonsaver  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)
target_include_directories(jsonsaver PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_include_directories (saver PRIVATE RAPID_JSON_INCLUDE_DIR)
