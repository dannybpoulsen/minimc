Find_Package (CGraph)


add_library (support STATIC   base64encode.cpp feedback.cpp timing.cpp)

if(ENABLE_SYMBOLIC)
target_sources (support PRIVATE smt.cpp) 
target_link_libraries (support PUBLIC smtlib)
endif (ENABLE_SYMBOLIC)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_link_libraries (support PUBLIC linuxsupport)
endif()	       

if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	add_subdirectory (x86_64)
	target_link_libraries (support PUBLIC x86_64support)
endif()

configure_file (
 "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

target_include_directories (support PUBLIC PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include PRIVATE ${CGRAPH_INCLUDE_DIRS} )

target_link_libraries (support PRIVATE ${CGRAPH_LIBRARIES} base64)
