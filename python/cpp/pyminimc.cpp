#include <pybind11/pybind11.h>

#include "support/smt.hpp"
#include <pybind11/stl.h>

namespace py = pybind11;

void addModelModule(py::module& m);
void addLoaderModule(py::module& m);
void addCPAModule(py::module& m);
void addAlgorithmsModule(py::module& m);

PYBIND11_MODULE(minimc, minimc) {
  addModelModule(minimc);
  addLoaderModule(minimc);
  addCPAModule(minimc);
  addAlgorithmsModule(minimc);

  auto submodule = minimc.def_submodule("smt");

  py::class_<MiniMC::Support::SMT::SMTDescr>(submodule, "SMTDescr")
      .def("name", &MiniMC::Support::SMT::SMTDescr::name)
      .def("description", &MiniMC::Support::SMT::SMTDescr::descr);
}
