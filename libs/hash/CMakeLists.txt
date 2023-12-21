project (hash CXX)
add_subdirectory (src)

install (TARGETS hash  			       EXPORT MiniMCTargets

		 			       FILE_SET HEADERS)
add_library(MiniMC::hash ALIAS hash)
