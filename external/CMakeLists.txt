
	include(ExternalProject)
	ExternalProject_Add(murmurhash
  	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/murmur
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/murmurhash
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND autoreconf -fi && ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/
	BUILD_COMMAND make
	INSTALL_COMMAND make install
	INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/
)

	ExternalProject_Get_Property(murmurhash install_dir)
	ExternalProject_Get_Property(murmurhash source_dir)
	
	add_library(murmur SHARED IMPORTED GLOBAL)	
	add_dependencies(murmur murmurhash)
 	set_property(TARGET murmur PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libmurmurhash.so.2.0.0)
	
	
	
	target_include_directories(murmur INTERFACE ${source_dir}) 
	install (DIRECTORY ${install_dir}/lib DESTINATION .)


add_subdirectory (smtlib)
