project (cpa CXX)

add_library(cpa_headers INTERFACE)
target_include_directories (cpa_headers INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include>)



add_subdirectory(internal) 

add_subdirectory(concrete) 
add_subdirectory(symbolic) 

#add_subdirectory(pathformula) 


add_subdirectory (src)

add_library(MiniMC::cpa ALIAS cpa)
