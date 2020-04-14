FIND_PATH(GTEST_INCLUDE_DIR NAMES gtest/gtest.h)
FIND_LIBRARY(GTEST_LIBRARY NAMES gtest)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GTEST DEFAULT_MSG GTEST_LIBRARY GTEST_INCLUDE_DIR)

IF(GTEST_FOUND)
	SET(GTEST_LIBRARIES ${GTEST_LIBRARY})
	SET(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIR})
ELSE(GTEST_FOUND)
	SET(GTEST_LIBRARIES)
	SET(GTEST_INCLUDE_DIRS)
ENDIF(GTEST_FOUND)