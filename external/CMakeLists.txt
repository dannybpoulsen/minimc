add_library (base64 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/base64/base64.cpp)
set_property(TARGET base64 PROPERTY POSITION_INDEPENDENT_CODE ON)

target_include_directories(base64 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/base64/)


add_subdirectory (smtlib)
