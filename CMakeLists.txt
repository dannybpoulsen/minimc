cmake_minimum_required(VERSION 3.13)
project(minimc)	
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules;${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 20)

option (ENABLE_SYMBOLIC "Enable Symbolic Representations" ON)
 option (ENABLE_PYTHON "Enable Python Binding" OFF)
option (MINIMC32PTr "Use 32Bit pointers" OFF)

enable_testing()

add_compile_options(-Wall -Wextra -Wpedantic)
if(MINIMC32PTr)
	add_compile_definition(MINIMC_32)
endif()

SET (ToolName minimc)
SET (VERSION_MAJOR 0)
SET (VERSION_MINOR "1")
execute_process(COMMAND
  git describe --always --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_HASH
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

include (package/rpath.cmake)

add_subdirectory (dependencies)

add_subdirectory (external)
add_subdirectory (libs)
add_subdirectory (bin)
add_subdirectory(tests)

if(ENABLE_PYTHON)	
  add_subdirectory (python)
endif()
include(package/package.cmake)
