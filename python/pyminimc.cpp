#include <pybind11/pybind11.h>

#include <pybind11/stl.h>
#include "support/smt.hpp"

namespace py = pybind11;

void addModelModule (py::module& m);
void addLoaderModule (py::module& m);
void addCPAModule (py::module& m);
void addAlgorithmsModule (py::module& m);



PYBIND11_MODULE(pyminimc, minimc) {
  addModelModule (minimc);
  addLoaderModule (minimc);
  addCPAModule (minimc);
  addAlgorithmsModule (minimc);

  auto submodule = minimc.def_submodule("smt");
  py::class_<MiniMC::Support::SMT::SMTDescr>  (submodule,"SMTDescr")
    .def("name",&MiniMC::Support::SMT::SMTDescr::name)
    .def("description",&MiniMC::Support::SMT::SMTDescr::descr)
    .def("useSolver",[](MiniMC::Support::SMT::SMTDescr& descr) {
      MiniMC::Support::SMT::setSMTSolver (&descr);
    }
      );

  submodule.
    def ("smtsolvers",[]() {
      std::vector<MiniMC::Support::SMT::SMTDescr> res;
      MiniMC::Support::SMT::getSMTBackends (std::back_inserter (res));
      return res;
    });
  
  
}





