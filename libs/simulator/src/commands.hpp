#ifndef _COMMANDS__
#define _COMMANDS__

#include "minimc/simulator/simulator.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/support/random.hpp"

#include <memory>

namespace MiniMC {
  namespace Simulator {
    class ShowStateCommand : public Command{
    public:
      ShowStateCommand (MiniMC::IO::ostream& os,MiniMC::Model::Program* prgm) : str(os),prgm(prgm) {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) 
	  MiniMC::CPA::CPAStateOutputter{*prgm}.output(simu->getState(),str);
	else
	  str << "No State" << MiniMC::IO::manipulator::endl;
	return true;
      }
    
    private:
      MiniMC::IO::ostream& str;
      MiniMC::Model::Program* prgm;
    };

    class ShowTransitionsCommand : public Command{
    public:
      ShowTransitionsCommand (MiniMC::IO::ostream& os) : str(os) {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) {
	  //MiniMC::CPA::CPAStateOutputter{*prgm}.output(simu->getState(),str);
	  for (auto t : transitions (simu->getState())) {
	    str << t << MiniMC::IO::manipulator::endl;
	  }
	}
	else
	  str << "No State" << MiniMC::IO::manipulator::endl;
	return true;
      }
    
    private:
      MiniMC::IO::ostream& str;
    };


    
    class StartSimulation : public Command{
    public:
      StartSimulation (MiniMC::Model::Program* prgm) : prgm(prgm) {}
      bool execute (Simulator* simu) override {
	simu->startSimulation (
			       MiniMC::CPA::InitialiseDescr{prgm->getEntryPoints (),
							    prgm->getHeapLayout (),
							    *prgm});
	return true;
      }
    
    private:
      MiniMC::Model::Program* prgm;
    };
    
    class StepSimulation : public Command{
    public:
      StepSimulation (MiniMC::IO::ostream& os, MiniMC::IO::Prompter& prompter) :os(os),prompter(prompter)  {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) {
	  std::vector<MiniMC::CPA::Transition> trans;
	  std::vector<std::string> trans_str;
	  
	  for (auto t : simu->getTransitions()) {
	    trans.push_back (t);
	    std::stringstream str;
	    str << t;
	    trans_str.push_back (str.str());
	  }
	  if (trans.size() == 0)
	    return false;
	  std::size_t selection = 0;
	  if (trans.size () > 1) {
	    selection = prompter.selectOption (trans_str);
	  }
	  
	  std::vector<MiniMC::CPA::State_ptr> states;
	  for (auto s : simu->step (trans.at (selection)))
	    states.push_back (s);
	  if (states.size () == 0)
	    return false;
	  else if (states.size() > 0) {
	    
	    simu->setState(states.at(MiniMC::Support::Random{}.uniform_int<std::size_t> (0,states.size()-1)));
	  }
	  
	    
	}
	return true;
      }
    
    private:
      MiniMC::IO::ostream& os;
      MiniMC::IO::Prompter& prompter;
    };

    class CommandBuilder {
    public:
      CommandBuilder (MiniMC::IO::ostream&os, MiniMC::Model::Program* prmg) :  os(os),prgm(prmg) {}
      void startSimulation () {
	cmd = std::make_unique<StartSimulation> (prgm);
      }

      void showState () {
	cmd = std::make_unique<ShowStateCommand> (os,prgm);
      }

      void showTransitions () {
	cmd = std::make_unique<ShowTransitionsCommand> (os);
      }
      
      auto get() {return std::move(cmd);}
      
    private:
      std::unique_ptr<Command> cmd {nullptr};
      MiniMC::IO::ostream& os;
      MiniMC::Model::Program* prgm;
    };
    
  }
}


#endif
