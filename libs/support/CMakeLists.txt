Find_Package (CGraph)

add_library (support SHARED jsongraph.cpp dotgraph.cpp)
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_sources (support PUBLIC $<TARGET_OBJECTS:linuxsupport>)
endif()	       

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
	target_sources (support PUBLIC $<TARGET_OBJECTS:x86_64support>)
endif()


target_include_directories (support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CGRAPH_INCLUDE_DIRS} )
target_link_libraries (support PRIVATE ${CGRAPH_LIBRARIES})

add_subdirectory (test)	
	
	
	
