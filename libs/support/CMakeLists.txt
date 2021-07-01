Find_Package (CGraph)

option (BIG_ENDIAN "Use Big endian encoding")

add_library (support STATIC  dotgraph.cpp base64encode.cpp feedback.cpp timing.cpp)

if(ENABLE_SYMBOLIC)
target_sources (support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/smt.cpp) 
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
 "${CMAKE_CURRENT_SOURCE_DIR}/include/support/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

target_include_directories (support PUBLIC PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include PRIVATE ${CGRAPH_INCLUDE_DIRS} )

target_link_libraries (support PRIVATE ${CGRAPH_LIBRARIES} base64)
if(BIG_ENDIAN)
    target_compile_options (support PUBLIC -DMINIMC_USE_BIG_ENDIAN)
endif(BIG_ENDIAN)
