

add_library (whileyloader SHARED loader.cpp  compiler.cpp)
target_link_libraries (whileyloader PUBLIC loaderinter model)
target_link_libraries (whileyloader PRIVATE Whiley::whiley)

target_include_directories (whileyloader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

INSTALL (TARGETS whileyloader)
add_library(MiniMC::whileyloader ALIAS whileyloader)
