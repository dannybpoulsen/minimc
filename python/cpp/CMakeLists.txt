

pybind11_add_module(pyminimc pyminimc.cpp model/model.cpp
			                  model/loader.cpp
					  model/modifications.cpp
					  cpa/cpa.cpp
					  algorithms.cpp
)

set_target_properties(pyminimc PROPERTIES OUTPUT_NAME minimc)

target_link_libraries(pyminimc
	PUBLIC 
minimclib  
)

