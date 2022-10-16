add_library (support STATIC feedback.cpp)


if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
	add_subdirectory (linux)
	target_link_libraries (support PUBLIC linuxsupport)
endif()	       


configure_file (
 "${CMAKE_CURRENT_SOURCE_DIR}/config.h.in"
 "${CMAKE_CURRENT_BINARY_DIR}/config.h"
)

target_include_directories (support PUBLIC ${CMAKE_CURRENT_BINARY_DIR} ${PROJECT_SOURCE_DIR}/include  )

