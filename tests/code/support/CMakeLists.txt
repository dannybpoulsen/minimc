include(GoogleTest)
find_package(GTest)
add_executable (output output.cpp)
add_executable (seq sequencer.cpp)
add_executable (gen generate.cpp)
add_executable (random random.cpp)
add_executable (base64e base64.cpp)
add_executable (dataflow dataflow.cpp)


target_link_libraries (base64e minimclib)
target_link_libraries (gen minimclib)
target_link_libraries (seq minimclib)
target_link_libraries (output minimclib)
target_link_libraries (random minimclib)
target_link_libraries (dataflow minimclib)

