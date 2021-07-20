find_package(pybind11 REQUIRED)


pybind11_add_module(pyminimc pyminimc.cpp model/model.cpp
			                  model/loader.cpp
					  model/modifications.cpp
)


target_link_libraries(pyminimc
PRIVATE	
    minimclib  
)

