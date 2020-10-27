
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
 	set_property(TARGET murmur PROPERTY IMPORTED_LOCATION ${install_dir}/lib/libmurmurhash.a)
	
	
	
	target_include_directories(murmur INTERFACE ${source_dir})


add_library (base64 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/base64/base64.cpp)
target_include_directories(base64 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/base64/)


add_subdirectory (smtlib)
