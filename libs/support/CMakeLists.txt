project (support CXX)
add_subdirectory (src)
add_subdirectory (tests)

install (TARGETS support 		 	 EXPORT MiniMCTargets FILE_SET HEADERS
)
add_library(MiniMC::support ALIAS support)
