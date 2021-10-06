cmake_minimum_required(VERSION 3.13)
project(minimc)	
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake_modules;${CMAKE_MODULE_PATH}")
set (CMAKE_CXX_STANDARD 17)

option (ENABLE_SYMBOLIC "Enable Symbolic Representations" ON)
option (ENABLE_PYTHON "Enable Symbolic Representations" OFF)


SET (ToolName minimc)
SET (VERSION_MAJOR 0)
SET (VERSION_MINOR "1")

execute_process(COMMAND
  git describe --always --dirty
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_HASH
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 
								   
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib:$ORIGIN/")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_subdirectory (external)
add_subdirectory (libs)
add_subdirectory (bin)
enable_testing()
add_subdirectory(tests)
	
add_subdirectory(package)	
add_subdirectory(docs)	
if(ENABLE_PYTHON)	
  add_subdirectory (python)
endif()
