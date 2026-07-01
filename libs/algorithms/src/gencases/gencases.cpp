#include "minimc/algorithms/gencases.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/smt/smt.hpp"

namespace MiniMC {
  namespace Algorithms {
    namespace GenCases {
      struct Progress  {
	Progress (std::size_t paths, std::size_t waiting) : paths(paths),waiting(waiting) {}
	std::size_t paths{0};
	std::size_t waiting{0};
      };
      
      inline MiniMC::IO::ostream& operator<< (MiniMC::IO::ostream& os, const Progress& p) {
	return os << MiniMC::Support::Localiser {"Paths / Waiting : %1% / %2%"}.format(p.paths,p.waiting);
      }
      
      
      struct TestCaseGenerator::Internal {
	Internal (MiniMC::Model::Program& prgm) : program(prgm) {}
	MiniMC::Model::Program& program;
      };

      TestCaseGenerator::TestCaseGenerator (MiniMC::Model::Program& prgm) {
	_internal = std::make_unique<Internal> (prgm);
      }
      TestCaseGenerator::~TestCaseGenerator () {}

      TestCaseGenResult TestCaseGenerator::generate  (MiniMC::Support::Messager& mess,
				   MiniMC::Model::Symbol& func,
				   MiniMC::Support::SMT::SMTDescr smt
						      ) {
	auto params = std::get<MiniMC::Model::Function_wptr>(func.getUserData()).lock()->getParameters();
	TestCaseGenResult res {{params.begin(),params.end()}};
	
	auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Symbolic> ();
	
	      
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
	std::size_t paths{0};
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
		paths++;
		res.addTestCase (std::move(values));
	      }
	      
	      else {
		waiting.push_back (state);
	      }
	      mess << MiniMC::Support::TProgress {Progress{paths, waiting.size ()}};
	    }
	  }
	}
	
	
	
	return res;
      }
      
      

    }
  }
}
