SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)


add_executable (minimc minimc.cpp pgraph.cpp enum.cpp plugin.cpp  smc.cpp mc.cpp)
target_link_libraries  (minimc minimclib ${Boost_LIBRARIES})
