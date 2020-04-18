find_package(Doxygen)

if (DOXYGEN_FOUND)
   set(SOURCE_LOCATION ${PROJECT_SOURCE_DIR}/libs)
   set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/doxyfile.in)
   set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
   configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT})
   add_custom_target( doc_doxygen 
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
endif(DOXYGEN_FOUND)
