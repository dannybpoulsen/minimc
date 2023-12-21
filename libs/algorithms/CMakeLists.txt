project (algorithms CXX)

add_subdirectory (src)
install (TARGETS algorithms
	EXPORT MiniMCTargets
	FILE_SET HEADERS)
add_library(MiniMC::algorithms ALIAS algorithms)
