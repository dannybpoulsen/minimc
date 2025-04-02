#include "minimc/simulator/simulator.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/support/random.hpp"

#include "commands.hpp"
#include "scanner.h"

namespace MiniMC {
  namespace Simulator {
    std::generator<MiniMC::CPA::State_ptr> Simulator::step (MiniMC::CPA::Transition t) const {
      for (auto s : transfer->doTransfer(getState(),t))
	co_yield s;
    }
    
    std::generator<MiniMC::CPA::Transition> Simulator::getTransitions () const {
      for (auto t : MiniMC::CPA::transitions (getState ()))
	     co_yield t;
    }

    void Simulator::startSimulation (const MiniMC::CPA::InitialiseDescr& desc) {
      state = cpa->makeInitialState(desc);
    }

    
    
    
    
    std::unique_ptr<Command> CommandParser::parse () {
      CommandBuilder builder{ostream,prompter,prgm};
      MiniMC::Model::ExpressionBuilder ebuilder;
      
      std::string line = prompter.readline();
      if (line.size())
	prompter.addHistory(line);
      
      std::stringstream stream;
      stream.str(line);
      MiniMC::Simulator::Scanner scanner {&stream};
      MiniMC::Simulator::Parser parser{scanner,builder,ebuilder,ostream};
      parser.parse ();
      auto cmd = builder.get();
      if (cmd)
	return cmd;
      
      
      return nullptr;
    }
    
    
  }
}
