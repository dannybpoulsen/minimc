add_compile_options(-Wall -Wextra )


add_subdirectory (model)
add_subdirectory (support)
add_subdirectory (hash)	
add_subdirectory (cpa)
add_subdirectory (algorithms)	
add_subdirectory (util)
add_subdirectory (vm)


add_library (minimclib SHARED)
target_link_libraries (minimclib PUBLIC model)
target_link_libraries (minimclib PUBLIC support)
target_link_libraries (minimclib PUBLIC hash)
target_link_libraries (minimclib PUBLIC vm)
target_link_libraries (minimclib PUBLIC cpa)
target_link_libraries (minimclib PUBLIC algorithms)
target_link_libraries (minimclib PUBLIC util)

