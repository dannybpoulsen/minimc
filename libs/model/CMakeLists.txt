project(model CXX)

add_subdirectory (isa)
add_subdirectory (src)	
add_subdirectory (tests)	

install(TARGETS modelisa FILE_SET HEADERS)
install(TARGETS model FILE_SET HEADERS)

	
