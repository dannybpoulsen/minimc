add_library (support SHARED feedback.cpp base64encode.cpp)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_link_libraries (support PUBLIC linuxsupport)
endif()	       


configure_file (
 "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

target_include_directories (support PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include  )
target_link_libraries (support PUBLIC host )

target_sources(support
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/support/exceptions.hpp
    	  ${PROJECT_SOURCE_DIR}/include/support/feedback.hpp
	  ${PROJECT_SOURCE_DIR}/include/support/localisation.hpp
	  ${PROJECT_SOURCE_DIR}/include/support/overload.hpp
	  ${PROJECT_SOURCE_DIR}/include/support/pipes.hpp
	  ${PROJECT_SOURCE_DIR}/include/support/workinglist.hpp
	  

)
