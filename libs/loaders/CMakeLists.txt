project (loaders)
add_subdirectory (interface)
add_subdirectory (llvm)
add_subdirectory (mmc2)


add_library (loader INTERFACE)
target_link_Libraries (loader INTERFACE loaderinter
		      	      		minimcloader
					llvmloader)

