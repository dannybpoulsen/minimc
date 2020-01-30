
	include(ExternalProject)
	ExternalProject_Add(murmurhash
  	SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/murmur
	PREFIX ${CMAKE_CURRENT_BINARY_DIR}/murmurhash
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND autoreconf -fi && ./configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/
	BUILD_COMMAND make
	INSTALL_COMMAND make install
	)

	add_library(murmur SHARED IMPORTED)	
	add_dependencies(murmur murmurhash)
 	set_property(TARGET murmur PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/lib/libmurmurhash.so.2.0.0)
	

	SET(MURMUR_LIBS  murmur)
	SET(MURMUR_INCLUDES ${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/include)
	install (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/murmurhash/install/lib DESTINATION .)
