project (binary CXX)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

if(ENABLE_SYMBOLIC)
    target_compile_options (support PUBLIC -DMINIMC_SYMBOLIC)
endif(ENABLE_SYMBOLIC)

#add_subdirectory (interpreter)

add_executable (minimc minimc.cpp mc.cpp plugin.cpp options.cpp convert.cpp symex.cpp)

target_link_libraries  (minimc minimclib ${Boost_LIBRARIES})
target_link_options (minimc PUBLIC "-Wl,--no-as-needed")

INSTALL (TARGETS minimc )
