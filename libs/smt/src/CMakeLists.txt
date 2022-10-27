

add_library (smt STATIC smt.cpp)

target_link_libraries (smt PUBLIC smtlib host)
target_include_directories (smt PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include  )
INSTALL (TARGETS smtlib)
