add_library (model SHARED cfg.cpp types.cpp variables.cpp program.cpp instructions.cpp  output.cpp symbol.cpp structurechecker expression_simplifier.cpp expression_simplifier_cast.cpp expression_simplifier_cast.cpp expression_simplifier_comp.cpp expression_simplifier_TAC.cpp VIL.cpp)
target_include_directories (model PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:/include> )
target_link_libraries (model PUBLIC util   host)

add_subdirectory (analysis)
add_subdirectory (modifications)

target_link_libraries (model PUBLIC modelisa
$<TARGET_OBJECTS:modifications> $<TARGET_OBJECTS:modelanalysis> )


				
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
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/valuevisitor.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/variables.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/analysis/loops.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/analysis/dominator.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/checkers/structuralchecker.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/func_inliner.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/loops.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/modifications.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/removephi.hpp
	  ${PROJECT_SOURCE_DIR}/include/minimc/model/modifications/splitasserts.hpp
	 	
	  )


