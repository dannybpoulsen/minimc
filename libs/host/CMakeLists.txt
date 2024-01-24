project (host CXX)
add_subdirectory (src)
add_subdirectory (tests)

install (TARGETS host EXPORT MiniMCTargets
		      FILE_SET HEADERS
		      
)
add_library(MiniMC::host ALIAS host)
