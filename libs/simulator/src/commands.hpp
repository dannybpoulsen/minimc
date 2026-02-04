#ifndef _COMMANDS__
#define _COMMANDS__

#include "minimc/simulator/simulator.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/smt/smt.hpp"

#include "minimc/support/random.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/algorithms/reachability.hpp"

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

    inline std::string symbol_descr_text (const MiniMC::Model::Symbol& s) {
      std::stringstream str;

      std::visit (MiniMC::Support::Overload {
	  [&str](const MiniMC::Model::Register_wptr&) {str << "Register";},
	    [&str](const MiniMC::Model::Function_wptr&) {str << "Function";},
	    [&str](const MiniMC::Model::Location_wptr&) {str << "Location";},
	    [&str](const MiniMC::Model::HeapBlock_wptr&) {str << "HeapObject";},
	    [&str](const std::monostate& ) {str << "??";}
	    }
	,s.getUserData());
      return str.str();
    }
    
    
    class ShowSymbols : public Command{
    public:
      ShowSymbols (std::size_t p, MiniMC::IO::ostream& os) : str(os),proc(p) {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) {
	  for (auto s : simu->getState().getLocationState().getLocation(0).getInfo().getFrame().local_and_parent_symbols()) {
	    str << s.getFullName () << " " << symbol_descr_text(s) << MiniMC::IO::manipulator::endl;
	  }
	  /*for (auto t : transitions (simu->getState())) {
	    str << t << MiniMC::IO::manipulator::endl;
	    }*/
	}
	else
	  str << "No State" << MiniMC::IO::manipulator::endl;
	return true;
      }
      
    private:
      MiniMC::IO::ostream& str;
      std::size_t proc;
    };
    
    class EvalExpression : public Command{
    public:
      EvalExpression (MiniMC::IO::ostream& os, const MiniMC::Model::Value_ptr& val,std::size_t p = 0, SMTLib::Context_ptr context = nullptr) : os(os),val(val),p(p),context(std::move(context)) {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) {
	  auto seval = simu->getState().getBuilder().buildValue (p,*val);
	  MiniMC::Model::Constant_ptr constant;
	  if (context)
	    constant = simu->getState().getConcretizer ({context})->evaluate(*seval);
	  else
	    constant = simu->getState().getConcretizer ()->evaluate(*seval);
	  os << *constant << MiniMC::IO::manipulator::endl;
	}
	else {
	  os << "No State" << MiniMC::IO::manipulator::endl;
	}
	return true;
      }
    
    private:
      MiniMC::IO::ostream& os;
      MiniMC::Model::Value_ptr val;
      std::size_t p;
      SMTLib::Context_ptr context;
      };


    class SymbEvalExpression : public Command{
    public:
      SymbEvalExpression (MiniMC::IO::ostream& os,const MiniMC::Model::Value_ptr& val,std::size_t p = 0) : os(os),val(val),p(p) {}
      bool execute(Simulator* simu) override {
        if (simu->hasState ()) {
	  auto seval = simu->getState().getBuilder().buildValue (p,*val);
	  os << *seval << MiniMC::IO::manipulator::endl;
	}
	else {
	  os << "No State" << MiniMC::IO::manipulator::endl ;
	}
	return true;
      }
      
    private:
      MiniMC::IO::ostream& os;
      MiniMC::Model::Value_ptr val;
      std::size_t p;
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

    class SymbolicExprSimulator : public Command{
    public:
      SymbolicExprSimulator () {}
      bool execute (Simulator* simu) override {
	auto cpa = MiniMC::CPA::makeCPA<MiniMC::CPA::CPAType::Symbolic> ();
	simu->updateCPA (cpa);
	return true;
      }
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

    class SearchCommand : public Command{
    public:
      SearchCommand (MiniMC::IO::ostream& os) : os(os) {}
      bool execute (Simulator* simu) override {
	if (simu->hasState ()) {
	  MiniMC::Support::Messager messager;
	  auto transfer = simu->getTransfer ();
	  auto state = simu->getState().copy();

	  MiniMC::Algorithms::Reachability::Reachability reach{transfer,messager};

	  auto goal = [](const MiniMC::CPA::State& state) {
	    auto& locationstate = state.getLocationState ();
	    auto procs = locationstate.nbOfProcesses ();
	    
	    for (std::size_t i = 0; i < procs; ++i) {
	      if (locationstate.getLocation (i).getInfo ().getFlags ().isSet (MiniMC::Model::Attributes::AssertViolated))
		return true;
	    }
	    
	    return false;
	  };
	  auto res = reach.search (*state,goal);
	  if (res.verdict () == MiniMC::Algorithms::Reachability::Verdict::Found) {
	    os << "Found a state\n";
	    simu->setState (res.foundState());
	    
	  }

	  else {
	    os << "No luck\n";
	  }
	  
	  return true;
	}
	return false;
      }
    
    private:
      MiniMC::IO::ostream& os;
    };
    
    
    class CommandBuilder {
    public:
      CommandBuilder (MiniMC::IO::ostream&os, MiniMC::IO::Prompter& p, MiniMC::Model::Program* prmg) :  os(os),prompter(p),prgm(prmg) {}
      void startSimulation () {
	cmd = std::make_unique<StartSimulation> (prgm);
      }

      void showState () {
	cmd = std::make_unique<ShowStateCommand> (os,prgm);
      }

      void showTransitions () {
	cmd = std::make_unique<ShowTransitionsCommand> (os);
      }
      
      void step () {
	cmd = std::make_unique<StepSimulation> (os,prompter);
      }

      void skip () {
	cmd = std::make_unique<Command> ();
      }

      void search () {
	cmd = std::make_unique<SearchCommand> (os);
      }

      void evalExpression (const MiniMC::Model::Value_ptr& v, const SMTLib::Context_ptr context = nullptr) {
	cmd = std::make_unique<EvalExpression> (os,v, 0,context);
      }
      
      void sevalExpression (const MiniMC::Model::Value_ptr& v) {
	cmd = std::make_unique<SymbEvalExpression> (os,v);
      }
      
      void makeSymbolic ( ) {
	cmd = std::make_unique<SymbolicExprSimulator> ();
      }
      
      void makeShowSymbols (std::size_t t) {
	cmd = std::make_unique<ShowSymbols> (t,os);
      }
      
      
      
      auto get() {return std::move(cmd);}
      
    private:
      std::unique_ptr<Command> cmd {nullptr};
      MiniMC::IO::ostream& os;
      MiniMC::IO::Prompter& prompter;
      MiniMC::Model::Program* prgm;
    };
    
  }
}


#endif
