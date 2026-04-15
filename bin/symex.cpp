#include <boost/program_options.hpp>

#include "minimc/support/localisation.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/loaders/loader.hpp"
#include "minimc/algorithms/gencases.hpp"
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
      po::options_description desc("Test Case Generation Options");
      desc.add_options()
	("gencases.function",po::value<std::string> (&locoptions.function)->default_value ("main"),"Set Function to analyse\n");
      op.add(desc);
      
    }
    
    MiniMC::Host::ExitCodes runCommand (MiniMC::Model::Program&& prgm, MiniMC::Support::Interaction& messager,const SetupOptions& sopt) {    
      auto func = prgm.getFunction (locoptions.function);
      
      MiniMC::Algorithms::GenCases::TestCaseGenerator generator {prgm};
      auto res = MiniMC::Support::AsyncExecutor{}.execute(messager,[&generator,&messager,&func,&sopt](){return generator.generate (messager.getMessager(),func,sopt.smt.selsmt);});
      
      for (auto& casee : res.cases ()) {
	auto reg_it = res.vars().begin ();
	auto val_it = casee.values().begin ();
	for (; reg_it != res.vars().end () && val_it != casee.values().end (); ++reg_it, ++val_it) {
	  messager.getMessager() << MiniMC::Support::TInfo<std::string> (MiniMC::Support::Localiser{"%1%            : %2%"}.format (**reg_it,**val_it));
	    //std::cout << **reg_it  << " : " << **val_it << std::endl;
	  
	}
	messager.getMessager() << MiniMC::Support::TInfo<std::string> ( "=====================");
      }
      
      return MiniMC::Host::ExitCodes::AllGood;
    }
    
    
    std::string getName () const override {return "gencases";}
    std::string getDescritpion () const override {return "Generate test cases for function ";}
    
  private:
    LocalOptions locoptions;
    
  };
} // namespace

static CommandRegistrar<MCCommand> mc_reg;
