
								   
#SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
SET(CMAKE_INSTALL_RPATH "$ORIGIN/../lib/:$ORIGIN/")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


SET(CPACK_GENERATOR "ZIP")
INSTALL (TARGETS minimclib DESTINATION lib)
INSTALL (TARGETS minimc DESTINATION bin)
if (ENABLE_SYMBOLIC)
   INSTALL (TARGETS  smtlib DESTINATION  lib)
  
endif(ENABLE_SYMBOLIC)


include(CPack)	
