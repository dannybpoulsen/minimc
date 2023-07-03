add_library (loader SHARED loader.cpp)
add_subdirectory (llvm)
#add_subdirectory (mmc)
add_subdirectory (mmc2)

target_link_libraries (loader PUBLIC
		      minimcloader
		      llvmloader )
target_include_directories(loader PUBLIC ${PROJECT_SOURCE_DIR}/include)

INSTALL (TARGETS loader)
