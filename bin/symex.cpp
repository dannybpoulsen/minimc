#include <boost/program_options.hpp>


#include "algorithms/successorgen.hpp"
#include "loaders/loader.hpp"
#include "algorithms/reachability/reachability.hpp"
#include "cpa/concrete.hpp"
#include "cpa/pathformula.hpp"
#include "cpa/state.hpp"

#include "options.hpp"
#include "plugin.hpp"

namespace po = boost::program_options;

namespace {

  struct LocalOptions {
    std::string function;
  };
  
  class MCCommand :public Command {
  public:

    void addOptions (po::options_description& op) {
      po::options_description desc("Symex Options");
      desc.add_options()
	("symex.function",po::value<std::string> (&locoptions.function)->default_value ("main"),"Set Function to analyse\n");
      op.add(desc);
      
    }
    
    MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Messager& messager,const SetupOptions& sopt) {    
      MiniMC::CPA::AnalysisBuilder cpa = makeCPABuilder (sopt);

      auto func = prgm.getFunction (locoptions.function);
      
      auto initstate = cpa.makeInitialState({
	  {func},
	  prgm.getHeapLayout (),
	    {},
	  prgm});

      std::vector<MiniMC::Model::Register_ptr> params = func->getParameters ();;
      std::vector<MiniMC::CPA::QueryExpr_ptr> params_sym; 
 
      for (auto p : params) {
	params_sym.push_back (initstate.dataStates()[0].getBuilder().buildValue (0,p));
      }
      

      auto transfer =  cpa.makeTransfer (prgm);
      std::vector<MiniMC::CPA::AnalysisState> waiting;
      waiting.push_back (initstate);
      while (waiting.size ()) {
	auto s = waiting.back();
	waiting.pop_back();
	MiniMC::Algorithms::SuccessorEnumerator enumerator {s,transfer};
	
	for (; enumerator; ++enumerator) {
	  auto state =   *enumerator;
	  auto concretizer = state.dataStates()[0].getConcretizer ();
	  if (concretizer->isFeasible () == MiniMC::CPA::Solver::Feasibility::Feasible) {
	    if (!state.getCFAState().isActive (0))  {
	      auto pit = params.begin ();
	      auto vit = params_sym.begin ();
	      for (; pit != params.end (); ++pit,++vit) {
		std::cerr << (*pit)->getSymbol().getFullName () << ":\t "<< * concretizer->evaluate (**vit) << std::endl;; 
	      }
	      std::cerr << "==============" << std::endl;
	    }
	    
	    else {
	      waiting.push_back (state);
	    }
	  }
	}
      }
      
            
      
      return MiniMC::Host::ExitCodes::AllGood;
    }
    
    
    std::string getName () const override {return "symex";}
    std::string getDescritpion () const override {return "Execute function symbolically. ";}
    
  private:
    MiniMC::CPA::AnalysisBuilder
    makeCPABuilder (const SetupOptions& sopt) {
      MiniMC::CPA::AnalysisBuilder cpa{std::make_shared<MiniMC::CPA::Location::CPA> ()};
      
      cpa.addDataCPA (std::make_shared<MiniMC::CPA::PathFormula::CPA>(sopt.smt.selsmt));
      return cpa;
    }
    LocalOptions locoptions;
    
  };
} // namespace

static CommandRegistrar<MCCommand> mc_reg;
