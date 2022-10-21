cmake_minimum_required(VERSION 3.13)
project(minimc VERSION 0.1.0)	
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules;${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 20)

option (ENABLE_SYMBOLIC "Enable Symbolic Representations" ON)
option (ENABLE_PYTHON "Enable Python Binding" OFF)
option (MINIMC32PTr "Use 32Bit pointers" OFF)

enable_testing()

add_compile_options(-Wall -Wextra -Wpedantic)
if(MINIMC32PTr)
	add_compile_definitions(MINIMC32)
endif()

include (dependencies/doctest.cmake)
include (dependencies/smtlib.cmake)
include (dependencies/llvm.cmake)
include (dependencies/xxhash.cmake)


SET (ToolName minimc)
SET (VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
SET (VERSION_MINOR ${PROJECT_VERSION_MINOR})
execute_process(COMMAND
  git describe --always --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_HASH
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

include(package/package.cmake)
include (package/rpath.cmake)


add_subdirectory (libs)
add_subdirectory (bin)
add_subdirectory (tests)

if(ENABLE_PYTHON)	
  add_subdirectory (python)
endif()


include(CPack)	
