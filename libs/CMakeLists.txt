add_subdirectory (model)
add_subdirectory (support)
add_subdirectory (loaders)	
add_subdirectory (hash)	
add_subdirectory (cpa)
add_subdirectory (algorithms)	
add_subdirectory (storage)


add_library (minimclib INTERFACE)
target_link_libraries (minimclib INTERFACE model)
target_link_libraries (minimclib INTERFACE loader)
target_link_libraries (minimclib INTERFACE support)
target_link_libraries (minimclib INTERFACE hash)
target_link_libraries (minimclib INTERFACE cpa)
target_link_libraries (minimclib INTERFACE algorithms)
target_link_libraries (minimclib INTERFACE storage)

