Find_Package (CGraph)

option (BIG_ENDIAN "Use Big endian encoding")

add_library (support STATIC  dotgraph.cpp)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_link_libraries (support PUBLIC linuxsupport)
endif()	       

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
	target_link_libraries (support PUBLIC x86_64support)
endif()


target_include_directories (support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CGRAPH_INCLUDE_DIRS} )
target_link_libraries (support PRIVATE ${CGRAPH_LIBRARIES})
if(BIG_ENDIAN)
    target_compile_options (support PUBLIC -DMINIMC_USE_BIG_ENDIAN)
endif(BIG_ENDIAN)
