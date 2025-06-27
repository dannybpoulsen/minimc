find_package (Python3 COMPONENTS Interpreter )

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/minimc/model")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/checkers")

add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instructions.inc"
	           OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instruction_constructors.inc"
		   OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/expr.inc"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/isa2cpp.py" "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml" "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instructions.inc"  "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instruction_constructors.inc" "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/expr.inc"	     
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/isa2cpp.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/isa.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/templates/expr_inc.jinja"
        DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/templates/instructions_inc.jinja"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/templates/instructions_constructors_inc.jinja"
	
    )

add_custom_target(generateISAHEADER ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instructions.inc")


add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/checkers/typechecker.hpp"
	           OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/typechecker.cpp"
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/typecheckgen.py" "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml" "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/checkers/typechecker.hpp" "${CMAKE_CURRENT_BINARY_DIR}/typechecker.cpp"	     
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/opcodes.yaml"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/typecheckgen.py"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/templates/typechecker_hpp.jinja"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/templates/typechecker_cpp.jinja"
        
    )

add_custom_target(generate_typechecker ALL DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/minimc/model/checkers/typechecker.hpp" )


add_library (modelisa STATIC ${CMAKE_CURRENT_BINARY_DIR}/typechecker.cpp)
add_dependencies(modelisa generateISAHEADER generate_typechecker)
target_include_directories (modelisa PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> $<BUILD_INTERFACE:/include>)
target_link_libraries(modelisa PUBLIC hash support util)

target_sources(modelisa
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${CMAKE_CURRENT_BINARY_DIR}	
    FILES ${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instruction_constructors.inc
	  ${CMAKE_CURRENT_BINARY_DIR}/minimc/model/instructions.inc
	  ${CMAKE_CURRENT_BINARY_DIR}/minimc/model/expr.inc
	  )	
