find_package (Python3 COMPONENTS Interpreter )

message (STATUS "${Python3_EXECUTABLE}")

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/model")
add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/model/instructions.inc"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/isa2cpp.py" "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml" "${CMAKE_CURRENT_BINARY_DIR}/model/instructions.inc" 	     
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml"
)

add_custom_target(generateISAHEADER ALL
				      DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/model/instructions.inc")

add_library (modelisa INTERFACE)
add_dependencies(modelisa generateISAHEADER)
target_include_directories (modelisa INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

