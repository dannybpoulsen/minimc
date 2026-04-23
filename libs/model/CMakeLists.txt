project(model CXX)

add_subdirectory (isa)
add_subdirectory (src)	
add_subdirectory (tests)	



add_library(MiniMC::model ALIAS model)

	
