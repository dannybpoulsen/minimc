add_library (host STATIC )
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
	target_link_libraries (host PUBLIC x86_64host)
endif()

add_subdirectory (tests)
