cmake_minimum_required(VERSION 3.5)
project(minimc)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules;${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 20)

add_subdirectory (libs)
add_subdirectory (exes)
enable_testing()
add_subdirectory(tests)
	
	
	
	
