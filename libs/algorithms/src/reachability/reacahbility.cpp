#include "algorithms/reachability/reachability.hpp"
#include "cpa/interface.hpp"
#include "algorithms/successorgen.hpp"
#include "support/localisation.hpp"
#include "storage/storage.hpp"
#include <algorithm>

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {

      inline std::ostream& operator<< (std::ostream& os, const Progress& p) {
	return os << MiniMC::Support::Localiser {"Waiting / Passed : %1% / %2%"}.format(p.waiting,p.passed);
      }
      
      
      template<class T>
      class WaitingList {
      public:
	virtual std::size_t size () const = 0;
	virtual T pop () = 0;
	virtual void insert (T ) = 0;
	operator bool () {return size ();}
      };

      template<class T>
      class BFSList :  public WaitingList<T> {
      public:
	
	std::size_t size () const override {return waiting.size ();}
	T pop () override {
	  auto state = std::move(waiting.back ());
	  waiting.pop_back ();
	  return state;
	}

	void insert (T t) {waiting.push_back (std::move(t));}
	
      private:
	std::list<T> waiting;
      };

      template<class T>
      class DFSList :  public WaitingList<T> {
      public:
	
	std::size_t size () const override {return waiting.size ();}
	T pop () override {
	  auto state = std::move(waiting.front ());
	  waiting.pop_front ();
	  return state;
	}

	void insert (T t) {waiting.push_front (std::move(t));}
	
	
      private:
	std::list<T> waiting;
      };
      
      StateStatus DefaultFilter (const MiniMC::CPA::AnalysisState& state) {
	for (auto& dstate : state.dataStates ()) {
	  auto solver = dstate.getConcretizer ();
	  switch (solver->isFeasible ()) {
	  case MiniMC::CPA::Solver::Feasibility::Feasible:
	  case MiniMC::CPA::Solver::Feasibility::Unknown:
	    break;
	  default:
	    return StateStatus::Discard;
	  }
	}

	return StateStatus::Keep;
      }

      struct Reachability::Internal {
	Internal (MiniMC::CPA::AnalysisTransfer transfer, std::unique_ptr<WaitingList<MiniMC::CPA::AnalysisState>>&& waiting = std::make_unique<DFSList<MiniMC::CPA::AnalysisState>> () ) : transfer(transfer),waiting(std::move(waiting)) {}
	MiniMC::CPA::AnalysisTransfer transfer;
	MiniMC::CPA::AnalysisState found;
	std::unique_ptr<WaitingList<MiniMC::CPA::AnalysisState>> waiting;
	std::size_t nbExploredStates;

	
      };

      
      MiniMC::CPA::AnalysisState Reachability::foundState () const {return _internal->found;}
      std::size_t Reachability::getNumberExploredStates () const {return _internal->nbExploredStates;}

      Reachability::~Reachability (){}
      Reachability::Reachability (MiniMC::CPA::AnalysisTransfer transfer) : _internal(std::make_unique<Internal> (transfer))  {}
      
      Verdict Reachability::search (MiniMC::Support::Messager& mess,const MiniMC::CPA::AnalysisState& state, GoalFunction goal,FilterFunction filter) {
	MiniMC::Storage::HashStorage storage;
	
        auto insert = [this,&storage,filter](auto& state) {  
	  if (filter(state) == StateStatus::Keep) {
	    auto ins = storage.insert (state);
	    if (ins) {
	      _internal->waiting->insert (state);
	    }
	  }
	};

	insert(state);
	while (*_internal->waiting) {
	  auto searchee = _internal->waiting->pop ();
	  
	  if (goal(searchee)) {
	    _internal->found = std::move(searchee);
	    _internal->nbExploredStates = storage.size ();
	    return Verdict::Found;
	  }
	  
	  if (searchee.getCFAState ().getLocationState().nbOfProcesses ()) {
	  
	    MiniMC::CPA::AnalysisState newstate;
	    MiniMC::Algorithms::TransitionEnumerator enumerator{searchee};
	    for (; enumerator; ++enumerator) {
	      if (_internal->transfer.Transfer (searchee,*enumerator,newstate)) {
		insert(newstate);
	      }
	    }
	  }
	  


	  mess << MiniMC::Support::TProgress {Progress{storage.size (), _internal->waiting->size ()}};
	}
	
	_internal->nbExploredStates = storage.size ();
	return Verdict::NotFound;
	
      }

      void Reachability::setSearchStrategy (SearchStrategy strat) {
	switch (strat) {
	  case SearchStrategy::DFS:
	    _internal->waiting = std::make_unique<DFSList<MiniMC::CPA::AnalysisState> > ();
	    break;
	case SearchStrategy::BFS:
	  _internal->waiting = std::make_unique<BFSList<MiniMC::CPA::AnalysisState>> ();
	 
	}
      }
	
      
    }
  }
}


