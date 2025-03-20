#ifndef _SIMULATOR__
#define _SIMULATOR__

#include "minimc/cpa/state.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/io/prompter.hpp"
#include "minimc/io/ostream.hpp"

#include <memory>
#include <generator>
namespace MiniMC {
  namespace Simulator {
    class Simulator {
    public:
      Simulator (MiniMC::CPA::TCPA_ptr cpa, MiniMC::Model::Program* p ) : cpa(cpa),prgm(p)  {
	transfer = cpa->makeTransfer(*prgm);
      }

      void startSimulation (const MiniMC::CPA::InitialiseDescr& desc);
      
      void setState (MiniMC::CPA::State_ptr nstate) {state = nstate;}; 
      auto& getState () const  {return *state;}
      bool hasState () const {return state != nullptr;} 
      std::generator<MiniMC::CPA::State_ptr> step (MiniMC::CPA::Transition) const;
      std::generator<MiniMC::CPA::Transition> getTransitions () const;
      auto getTransfer () const {return transfer;}
    private:
      MiniMC::CPA::State_ptr state{nullptr};
      MiniMC::CPA::TCPA_ptr cpa;
      MiniMC::CPA::Transferer_ptr transfer;
      MiniMC::Model::Program* prgm;
    };

    class Command {
    public:
      virtual bool execute (Simulator* simu) {return true;}
    };

    
    class CommandParser {
    public:
      CommandParser (MiniMC::IO::ostream& os, MiniMC::IO::Prompter& prompter, MiniMC::Model::Program* prgm) : ostream(os),prompter(prompter),prgm(prgm) {}
      std::unique_ptr<Command> parse (); 
    private:
      MiniMC::IO::ostream& ostream;
      MiniMC::IO::Prompter& prompter;
      MiniMC::Model::Program* prgm;
    };
    
    
  }
}

#endif
