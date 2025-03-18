project(simulator CXX)

add_subdirectory (src)	


install (TARGETS simulator EXPORT MiniMCTargets FILE_SET HEADERS)
add_library(MiniMC::simulator ALIAS simulator)

	
