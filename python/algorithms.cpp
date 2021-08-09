#include <sstream>
#include <pybind11/pybind11.h>
#include <pybind11/functional.h>


#include "algorithms/simulationmanager.hpp"
namespace py = pybind11;

void addAlgorithmsModule (py::module& m) {
  auto submodule = m.def_submodule("algorithms");

  py::class_<MiniMC::Algorithms::SimManagerOptions> (submodule,"SimManagerOptions")
    .def(py::init<> ())
    .def_readwrite ("storage",&MiniMC::Algorithms::SimManagerOptions::storage)
    .def_readwrite ("storer",&MiniMC::Algorithms::SimManagerOptions::storer)
    .def_readwrite ("joiner",&MiniMC::Algorithms::SimManagerOptions::joiner)
    .def_readwrite ("transfer",&MiniMC::Algorithms::SimManagerOptions::transfer);

  py::class_<MiniMC::Algorithms::SearchOptions> (submodule,"SearchOptions")
    .def(py::init<> ())
    .def_readwrite ("filter",&MiniMC::Algorithms::SearchOptions::filter)
    .def_readwrite ("delay",&MiniMC::Algorithms::SearchOptions::delay)
    .def_readwrite ("goal",&MiniMC::Algorithms::SearchOptions::goal);

  py::class_<MiniMC::Algorithms::SimulationManager> (submodule,"SimulationManager")
    .def(py::init<MiniMC::Algorithms::SimManagerOptions> ())
    .def ("search",&MiniMC::Algorithms::SimulationManager::reachabilitySearch)
    .def ("insertState",[](MiniMC::Algorithms::SimulationManager& man,
			   MiniMC::CPA::State_ptr& state) {
      man.insert (state);
    }	  );
}
