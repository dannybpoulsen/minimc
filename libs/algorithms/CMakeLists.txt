add_library (algorithms STATIC)
target_link_libraries (algorithms PUBLIC support cpa)
target_precompile_headers (algorithms PRIVATE ${PROJECT_SOURCE_DIR}/include/algorithms/enumstates.hpp
			                      ${PROJECT_SOURCE_DIR}/include/algorithms/simulationmanager.hpp
)

target_include_directories (algorithms PUBLIC ${PROJECT_SOURCE_DIR}/include)
