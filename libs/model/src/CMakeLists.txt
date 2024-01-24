add_library (model SHARED cfg.cpp types.cpp variables.cpp program.cpp instructions.cpp  output.cpp symbol.cpp)
target_include_directories (model PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include> )
target_link_libraries (model PUBLIC util support host)

add_subdirectory (analysis)
add_subdirectory (checkers)
add_subdirectory (modifications)

target_link_libraries (model PUBLIC modelisa $<TARGET_OBJECTS:checkers> $<TARGET_OBJECTS:modifications> $<TARGET_OBJECTS:modelanalysis> )



target_sources(model
    PUBLIC FILE_SET HEADERS 
    BASE_DIRS ${PROJECT_SOURCE_DIR}/include	
    FILES ${PROJECT_SOURCE_DIR}/include/minimc/model/cfg.hpp
    	  ${PROJECT_SOURCE_DIR}/include/minimc/model/array.hpp
      	  ${PROJECT_SOURCE_DIR}/include/minimc/model/builder.hpp
      	  ${PROJECT_SOURCE_DIR}/include/minimc/model/edge.hpp
       	  ${PROJECT_SOURCE_DIR}/include/minimc/model/heaplayout.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/instructions.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/location.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/source.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/symbol.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/types.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/utils.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/valuevisitor.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/variables.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/analysis/loops.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/analysis/dominator.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/checkers/structuralchecker.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/checkers/typechecker.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/expandnondet.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/func_inliner.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/loops.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/modifications.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/removephi.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/splitasserts.hpp
	 	
	  )


