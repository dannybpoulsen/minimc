add_library (model SHARED cfg.cpp types.cpp variables.cpp program.cpp instructions.cpp  output.cpp symbol.cpp)
target_include_directories (model PUBLIC ${PROJECT_SOURCE_DIR}/include )
target_link_libraries (model util support host)

add_subdirectory (analysis)
add_subdirectory (checkers)
add_subdirectory (modifications)

target_link_libraries (model modelisa checkers modifications modelanalysis )



target_sources(model
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/model/cfg.hpp
    	  ${PROJECT_SOURCE_DIR}/include/model/array.hpp
      	  ${PROJECT_SOURCE_DIR}/include/model/builder.hpp
      	  ${PROJECT_SOURCE_DIR}/include/model/edge.hpp
       	  ${PROJECT_SOURCE_DIR}/include/model/heaplayout.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/instructions.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/location.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/source.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/symbol.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/types.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/utils.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/valuevisitor.hpp
	  ${PROJECT_SOURCE_DIR}/include/model/variables.hpp
	  
	  )

 	
target_sources(model
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_BINARY_DIR}/isa	
    FILES ${PROJECT_BINARY_DIR}/isa/model/instruction_constructors.inc
	  ${PROJECT_BINARY_DIR}/isa/model/instructions.inc
	  )
