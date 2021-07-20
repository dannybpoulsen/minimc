#include <sstream>
#include <pybind11/pybind11.h>
#include "model/modifications/splitasserts.hpp"
#include "model/modifications/insertboolcasts.hpp"
#include "model/modifications/loops.hpp"
#include "model/modifications/simplify_cfg.hpp"
#include "model/modifications/func_inliner.hpp"
#include "model/modifications/lower_guards.hpp"


namespace py = pybind11;



void addModificationsModule (py::module& m) {
  namespace mcmod =  MiniMC::Model::Modifications;
  auto submodule = m.def_submodule("modifications");
  
  py::class_<mcmod::SplitAsserts> (submodule,"SplitAsserts")
    .def (py::init<> ())
    .def ("runFunction",&mcmod::SplitAsserts::runFunction);
  
  py::class_<mcmod::InsertBoolCasts> (submodule,"InsertBoolCasts")
    .def (py::init<> ())
    .def ("runFunction",&mcmod::InsertBoolCasts::runFunction);

  py::class_<mcmod::UnrollLoops> (submodule,"UnrollLoops")
    .def (py::init<std::size_t> ())
    .def ("runFunction",&mcmod::UnrollLoops::runFunction);
  
  py::class_<mcmod::SimplifyCFG> (submodule,"SimplifyCFG")
    .def (py::init<> ())
    .def ("runFunction",&mcmod::SimplifyCFG::runFunction);

  py::class_<mcmod::InlineFunctions> (submodule,"InlineFunctions")
    .def (py::init<std::size_t> ())
    .def ("runFunction",&mcmod::InlineFunctions::runFunction);

  py::class_<mcmod::LowerGuards> (submodule,"LowerGuards")
    .def (py::init<> ())
    .def ("runFunction",&mcmod::LowerGuards::runFunction);
  
  
  
  
}
