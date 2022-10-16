include(FetchContent)
FetchContent_Declare(xxhash
	URL https://github.com/Cyan4973/xxHash/archive/refs/tags/v0.8.0.tar.gz
	URL_HASH SHA512=c3973b3c98bad44e1d8687ab4f9461aecd1c071bb3d320537a4c50fb7301edd13e990bab48cc6e5ca30536a814c8fa8cac24ceb1803a7e8eca30ef73d449373e 
	)
FetchContent_GetProperties(xxhash)
if(NOT xxhash_POPULATED)
	 FetchContent_Populate(xxhash)
	 add_subdirectory (${xxhash_SOURCE_DIR}/cmake_unofficial ${xxhash_BINARY_DIR} EXCLUDE_FROM_ALL)
	 add_library (xxhash_imp INTERFACE)
	 target_link_libraries (xxhash_imp INTERFACE xxhash)
	 set(XXHash_LIBRARIES xxhash_imp)
		 
endif()

