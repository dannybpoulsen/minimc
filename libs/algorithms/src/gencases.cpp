#include "algorithms/gencases.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/location.hpp"
#include "cpa/successorgen.hpp"
#include "smt/smt.hpp"

namespace MiniMC {
  namespace Algorithms {
    namespace GenCases {
      struct TestCaseGenerator::Internal {
	Internal (MiniMC::Model::Program& prgm) : program(prgm) {}
	MiniMC::Model::Program& program;
      };

      TestCaseGenerator::TestCaseGenerator (MiniMC::Model::Program& prgm) {
	_internal = std::make_unique<Internal> (prgm);
      }
      TestCaseGenerator::~TestCaseGenerator () {}

      TestCaseGenResult TestCaseGenerator::generate  (MiniMC::Support::Messager& mess,
				   MiniMC::Model::Function_ptr& func,
				   MiniMC::Support::SMT::SMTDescr smt
						      ) {
	TestCaseGenResult res {func->getParameters()};
	MiniMC::CPA::AnalysisBuilder cpa{std::make_shared<MiniMC::CPA::Location::CPA> ()};
	
	cpa.addDataCPA (std::make_shared<MiniMC::CPA::PathFormula::CPA>(smt));
      
	
	      
	auto initstate = cpa.makeInitialState({
	    {func},
	    {},
	    {},
	    _internal->program}
	  );
	
	std::vector<MiniMC::CPA::QueryExpr_ptr> params_sym; 
      
	for (auto p : res.vars ()) {
	  params_sym.push_back (initstate.dataStates()[0].getBuilder().buildValue (0,p));
	}
	
	
	auto transfer =  cpa.makeTransfer (_internal->program);
	std::vector<MiniMC::CPA::AnalysisState> waiting;
	waiting.push_back (initstate);
	while (waiting.size ()) {
	  auto s = waiting.back();
	  waiting.pop_back();
	  MiniMC::CPA::SuccessorEnumerator enumerator {s,transfer};
	  
	  for (; enumerator; ++enumerator) {
	    auto state =   *enumerator;
	    auto concretizer = state.dataStates()[0].getConcretizer ();
	    if (concretizer->isFeasible () == MiniMC::CPA::Solver::Feasibility::Feasible) {
	      if (!state.getCFAState().isActive (0))  {
		std::vector<MiniMC::Model::Value_ptr> values;
		auto inserter = std::back_inserter (values);
		std::for_each (params_sym.begin (),params_sym.end (),[&inserter,&concretizer](auto& sym_val){inserter = concretizer->evaluate (*sym_val);});
		res.addTestCase (std::move(values));
	      }
	      
	      else {
		waiting.push_back (state);
	      }
	    }
	  }
	  }
	
	
	
	return res;
      }
      
      

    }
  }
}
