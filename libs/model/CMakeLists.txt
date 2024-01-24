project(model CXX)

add_subdirectory (isa)
add_subdirectory (src)	
add_subdirectory (tests)	

install(TARGETS modelisa 			 EXPORT MiniMCTargets
						  FILE_SET HEADERS
)
install(TARGETS model 	EXPORT MiniMCTargets
			FILE_SET HEADERS
)
add_library(MiniMC::model ALIAS model)

	
