#include "minimc/algorithms/reachability.hpp"
#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/successorgen.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/support/localisation.hpp"
#include "minimc/storage/storage.hpp"
#include <algorithm>
#include <iostream>
#include <future>

using namespace std::chrono_literals;

using namespace std::chrono_literals;

namespace MiniMC {
  namespace Algorithms {
    namespace Reachability {

      inline MiniMC::IO::ostream& operator<< (MiniMC::IO::ostream& os, const Progress& p) {
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
	  auto state = std::move(waiting.front ());
	  waiting.pop_front ();
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
      
      StateStatus DefaultFilter (const MiniMC::CPA::State& state) {
	
	auto solver = state.getConcretizer ();
	auto res = solver->isFeasible ();
	
	switch (res) {
	case MiniMC::CPA::Solver::Feasibility::Feasible:
	case MiniMC::CPA::Solver::Feasibility::Unknown:
	  break;
	default:
	  return StateStatus::Discard;
	}
	
      
	return StateStatus::Keep;
      }

      struct Reachability::Internal {
	Internal (std::unique_ptr<WaitingList<MiniMC::CPA::State_ptr>>&& waiting,GoalFunction goal,FilterFunction filter ) : waiting(std::move(waiting)),storage(std::make_unique<MiniMC::Storage::HashStorage<MiniMC::CPA::State>> ()),goal(goal),filter(filter) {}
	
	std::unique_ptr<WaitingList<MiniMC::CPA::State_ptr>> waiting;
	std::unique_ptr<MiniMC::Storage::Store<MiniMC::CPA::State>> storage;
	GoalFunction goal;
	FilterFunction filter;
      };

      
      Reachability::~Reachability (){}
      Reachability::Reachability (MiniMC::CPA::Transferer_ptr transfer, MiniMC::Support::Interaction mess) : mess(mess),transfer(transfer)  {}
      
      std::unique_ptr<WaitingList<MiniMC::CPA::State_ptr>> getSearchStrategy (SearchStrategy strat) {
	switch (strat) {
	case SearchStrategy::DFS:
	  return std::make_unique<DFSList<MiniMC::CPA::State_ptr> > ();
	  break;
	case SearchStrategy::BFS:
	  return std::make_unique<BFSList<MiniMC::CPA::State_ptr>> ();
	default:
	  std::unreachable();
	}
      }
	
      
      Result Reachability::search (const MiniMC::CPA::State& state, GoalFunction goal,FilterFunction filter, SearchStrategy strat) {
	_internal = std::make_unique<Internal> (getSearchStrategy (strat),goal,filter); 
	auto nstate = state.copy();
	_internal->storage->insert (*nstate);
	_internal->waiting->insert (nstate);
	return search ();
      }

      [[nodiscard]] Result Reachability::continueSearch () {
	if (_internal == nullptr) {
	  return Result{0};
	}

	return search ();
      }
	

      Result Reachability::search () {
	auto insert = [this](auto&& state) {  
	  auto filterres = _internal->filter(*state);
	  if (filterres == StateStatus::Keep) {
	    auto ins = _internal->storage->insert (*state);
	    if (ins) {
	      _internal->waiting->insert (state);
	    }
	  }
	};
	mess.getMessager() << MiniMC::Support::TProgress {Progress{_internal->storage->size (), _internal->waiting->size ()}};
	
	while (*_internal->waiting && mess.getInteractor().curEvent () == MiniMC::Support::Event::Continue) {
	  auto searchee = _internal->waiting->pop ();
	  if (_internal->goal(*searchee)) {
	    return Result {std::move(searchee),_internal->storage->size()};
	  }
	  
	  
	  for (auto newstate : successors (*searchee,*transfer))
	    insert(newstate);
	  
	  
	  
	  mess.getMessager() << MiniMC::Support::TProgress {Progress{_internal->storage->size (), _internal->waiting->size ()}};
	  
	  
	  
	}
	
	return Result{_internal->storage->size ()};	
	
      }
      
      
      
    }
  }
}


