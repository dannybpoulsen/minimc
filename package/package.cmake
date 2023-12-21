include (GNUInstallDirs)

# when building, don't use the install RPATH already
# (but later on when installing)

SET(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${GIT_HASH}")


set (${MINIMC_INSTALL_BINDIR} ${CMAKE_INSTALL_BINDIR})


set (MINIMC_INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR})
set (CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set (CMAKE_INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})



