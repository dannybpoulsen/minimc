project(libs)
add_compile_options(-Wall -Wextra -fconcepts-diagnostics-depth=3)


add_subdirectory (io)
add_subdirectory (simulator)
add_subdirectory (host)
add_subdirectory (model)
add_subdirectory (loaders)
add_subdirectory (support)
add_subdirectory (hash)	
add_subdirectory (values)
add_subdirectory (vm)
add_subdirectory (cpa)
add_subdirectory (storage)
add_subdirectory (algorithms)	
add_subdirectory (util)
add_subdirectory (smt)



add_library(minimcfront SHARED dummy.cpp)
target_link_Libraries(minimcfront PUBLIC cpa support loader algorithms)
target_include_directories(minimcfront PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include>
)

add_library (minimclib INTERFACE)
target_link_libraries (minimclib INTERFACE io)
target_link_libraries (minimclib INTERFACE model)
target_link_libraries (minimclib INTERFACE loader)
target_link_libraries (minimclib INTERFACE support)
target_link_libraries (minimclib INTERFACE hash)
target_link_libraries (minimclib INTERFACE vm)
target_link_libraries (minimclib INTERFACE cpa)
target_link_libraries (minimclib INTERFACE algorithms)
target_link_libraries (minimclib INTERFACE util)
target_link_libraries (minimclib INTERFACE host)
target_link_libraries (minimclib INTERFACE smt)
target_link_libraries (minimclib INTERFACE values)
target_link_libraries (minimclib INTERFACE simulator)
target_link_libraries (minimclib INTERFACE minimcfront
)

target_sources(minimcfront
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/minimc.hpp
)

install (TARGETS host smt model modelisa cpa simulator minimcfront loader loaderinter minimcloader llvmloader whileyloader llvm13 EXPORT MiniMCTargets FILE_SET HEADERS)


install(
  EXPORT MiniMCTargets
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/minimc
  NAMESPACE MyPack::
  )
