#include <pybind11/pybind11.h>

#include "cpa/compound.hpp"
#include "cpa/concrete.hpp"
#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/state.hpp"
#include "support/types.hpp"
#include <sstream>

namespace py = pybind11;

void addCPAModule(py::module& m) {
  auto submodule = m.def_submodule("cpa");

  submodule
      .def("Location", []() { return MiniMC::CPA::CPA_ptr(new MiniMC::CPA::Location::CPA()); })

      .def("Concrete", []() { return MiniMC::CPA::CPA_ptr(new MiniMC::CPA::Compounds::CPA(std::initializer_list<MiniMC::CPA::CPA_ptr>({std::make_shared<MiniMC::CPA::Location::CPA>(),
		  std::make_shared<MiniMC::CPA::Concrete::CPA>()}))); })

  ;

  py::class_<MiniMC::CPA::State, MiniMC::CPA::State_ptr> state(submodule, "State");
  state
      .def("assertViolated", &MiniMC::CPA::State::assertViolated)
      .def("getConcretizer", &MiniMC::CPA::State::getConcretizer)
      .def("symbEvaluate",&MiniMC::CPA::State::symbEvaluate)
      .def("__str__", [](const MiniMC::CPA::State_ptr& state) {
        std::stringstream str;
        state->output(str);
        return str.str();
      });


  py::class_<MiniMC::CPA::ByteVectorExpr, MiniMC::CPA::ByteVectorExpr_ptr> bytevec(submodule, "ByteVectorExpr");
  bytevec
    .def("size", &MiniMC::CPA::ByteVectorExpr::getSize)
  .def("__str__", [](const MiniMC::CPA::ByteVectorExpr& bvec) {
        std::stringstream str;
        bvec.output(str);
        return str.str();
    });
  
    
  py::class_<MiniMC::CPA::Solver, MiniMC::CPA::Solver_ptr> concr(submodule, "Solver");
  

  py::class_<MiniMC::CPA::StateQuery, MiniMC::CPA::StateQuery_ptr>(submodule, "StateQuery")
      .def("makeInitialState", &MiniMC::CPA::StateQuery::makeInitialState);

  py::class_<MiniMC::CPA::Transferer, MiniMC::CPA::Transferer_ptr>(submodule, "Transfer")
      .def("doTransfer", &MiniMC::CPA::Transferer::doTransfer);

  py::class_<MiniMC::CPA::Joiner, MiniMC::CPA::Joiner_ptr>(submodule, "Joiner")
      .def("doJoin", &MiniMC::CPA::Joiner::doJoin)
      .def("covers", &MiniMC::CPA::Joiner::covers);

  py::class_<MiniMC::CPA::IStorer, MiniMC::CPA::Storer_ptr>(submodule, "Store");

  py::class_<MiniMC::CPA::ICPA, MiniMC::CPA::CPA_ptr>(submodule, "CPA")
      .def("makeQuery", &MiniMC::CPA::ICPA::makeQuery)
      .def("makeTransfer", &MiniMC::CPA::ICPA::makeTransfer)
      .def("makeStore", &MiniMC::CPA::ICPA::makeStore)
      .def("makeJoin", &MiniMC::CPA::ICPA::makeJoin);
}
