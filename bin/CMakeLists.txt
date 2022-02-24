
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

if(ENABLE_SYMBOLIC)
    target_compile_options (support PUBLIC -DMINIMC_SYMBOLIC)
endif(ENABLE_SYMBOLIC)


add_executable (minimc minimc.cpp enum.cpp pgraph.cpp smc.cpp mc.cpp plugin.cpp )

target_link_libraries  (minimc minimclib ${Boost_LIBRARIES})
