add_library (host INTERFACE )
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
	target_link_libraries (host INTERFACE x86_64host)

endif()
