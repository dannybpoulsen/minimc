#include <sstream>
#include <string>
#include <pybind11/pybind11.h>

#include "model/types.hpp"
#include "model/variables.hpp"
#include "loaders/loader.hpp"

namespace py = pybind11;

void addLLVMLoaderModule (py::module& m) {
  auto llvmMod =   m.def_submodule("llvm");
  llvmMod
    .def ("from_file",
	  [](const std::string& input,
	     MiniMC::Model::TypeFactory_ptr tfac,
	     MiniMC::Model::ConstantFactory_ptr cfac) {
	    if (tfac == nullptr)
	      tfac = std::shared_ptr<MiniMC::Model::TypeFactory> (new MiniMC::Model::TypeFactory64 ());
	    if (cfac == nullptr)
	      cfac = std::shared_ptr<MiniMC::Model::ConstantFactory> (new MiniMC::Model::ConstantFactory64 ());
	    
	    return MiniMC::Loaders::loadFromFile<MiniMC::Loaders::Type::LLVM> (input, typename MiniMC::Loaders::OptionsLoad<MiniMC::Loaders::Type::LLVM>::Opt {.tfactory = tfac,
																			       .cfactory =cfac});
	    
	  },
	  py::arg("input") = "main.ll",
	  py::arg("tfac") = nullptr,
	  py::arg("cfac") = nullptr
	  );
	  
  
}

void addLoaderModule (py::module& m) {
  auto submodule = m.def_submodule ("loaders");
  addLLVMLoaderModule (submodule);
}
