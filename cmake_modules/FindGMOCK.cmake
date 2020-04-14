FIND_PATH(GMOCK_INCLUDE_DIR NAMES gmock/gmock.h)
FIND_LIBRARY(GMOCK_LIBRARY NAMES gmock)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GMOCK DEFAULT_MSG GMOCK_LIBRARY GMOCK_INCLUDE_DIR)

IF(GMOCK_FOUND)
	SET(GMOCK_LIBRARIES ${GTEST_LIBRARY})
	SET(GMOCK_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
ELSE(GMOCK_FOUND)
	SET(GMOCK_LIBRARIES)
	SET(GMOCK_INCLUDE_DIRS)
ENDIF(GMOCK_FOUND)