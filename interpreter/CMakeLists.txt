
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)

if(ENABLE_SYMBOLIC)
    target_compile_options (support PUBLIC -DMINIMC_SYMBOLIC)
endif(ENABLE_SYMBOLIC)


add_executable (mini-interpreter miniinterpreter.cpp)

target_link_libraries  (mini-interpreter minimclib ${Boost_LIBRARIES})
