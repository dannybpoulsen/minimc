#include "minimc/algorithms/gencases.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/pathformula.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/smt/smt.hpp"

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
	/*MiniMC::CPA::AnalysisBuilder cpa;
	cpa.add<MiniMC::CPA::CPAType::Pathformula>(smt);
	*/
	auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Pathformula> (smt);
	
	      
	auto initstate = cpa->makeInitialState({
	    {func},
	    {},
	    _internal->program}
	  );
	
	std::vector<MiniMC::CPA::QueryExpr_ptr> params_sym; 
      
	for (auto& p : res.vars ()) {
	  params_sym.push_back (initstate->getBuilder().buildValue (0,*p));
	}
	
	
	auto transfer =  cpa->makeTransfer (_internal->program);
	std::vector<MiniMC::CPA::State_ptr> waiting;
	waiting.push_back (initstate);
	while (waiting.size ()) {
	  auto s = std::move(waiting.back());
	  waiting.pop_back();
	  
	  for (auto state : MiniMC::CPA::successors (*s,*transfer)) {
	    auto concretizer = state->getConcretizer ();
	    if (concretizer->isFeasible () == MiniMC::CPA::Solver::Feasibility::Feasible) {
	      if (!state->getLocationState().isActive (0))  {
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
