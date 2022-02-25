include(GNUInstallDirs)


# when building, don't use the install RPATH already
# (but later on when installing)

SET(CPACK_GENERATOR "ZIP")
INSTALL (TARGETS minimclib DESTINATION ${CMAKE_INSTALL_LIBDIR})
INSTALL (TARGETS minimc DESTINATION ${CMAKE_INSTALL_BINDIR})
if (ENABLE_SYMBOLIC)
   INSTALL (TARGETS  smtlib DESTINATION  ${CMAKE_INSTALL_LIBDIR})  
endif(ENABLE_SYMBOLIC)

if (ENABLE_PYTHON)
     find_package (Python3 REQUIRED)
     if (NOT CMAKE_INSTALL_PYTHON_LIBDIR)
      set(CMAKE_INSTALL_PYTHON_LIBDIR "${CMAKE_INSTALL_LIBDIR}/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}/site-packages")
      endif ()
      set_target_properties(pyminimc
                           PROPERTIES INSTALL_RPATH "$ORIGIN:$ORIGIN/../../.."
      			   )

      INSTALL (TARGETS pyminimc
               COMPONENT PYTHON  
	       DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR}/minimc)
      INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/python/minimc/ DESTINATION ${CMAKE_INSTALL_PYTHON_LIBDIR}/minimc)
endif(ENABLE_PYTHON)



include(CPack)	
