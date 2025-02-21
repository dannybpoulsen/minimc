add_subdirectory(concrete)

add_library(values INTERFACE)
target_link_libraries(values INTERFACE vm_concrete)
