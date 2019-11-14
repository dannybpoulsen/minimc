SET(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost COMPONENTS program_options REQUIRED)


add_executable (succ succegenerator.cpp)
target_link_libraries  (succ minimclib)

add_executable (pgraph pgraph.cpp)
target_link_libraries  (pgraph minimclib ${Boost_LIBRARIES})
