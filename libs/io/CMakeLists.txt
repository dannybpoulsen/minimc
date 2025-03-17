project(io CXX)

add_subdirectory (src)	


install (TARGETS io EXPORT MiniMCTargets FILE_SET HEADERS
)
add_library(MiniMC::io ALIAS io)

	
