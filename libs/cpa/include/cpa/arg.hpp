
#include <vector>

#include "cpa/state.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "model/cfg.hpp"
#include "hash/hashing.hpp"


namespace MiniMC {
  namespace CPA {
    namespace ARG {
      class State :public MiniMC::CPA::State {
      public:
	State (MiniMC::CPA::State_ptr wrapped) : wrappedState(wrapped) {}
	State (const State& s) :wrappedState(s.wrappedState), parents(s.parents) {}  
	
	virtual std::ostream& output (std::ostream& os) const {return wrappedState->output (os);}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {return wrappedState->hash (seed);}
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return std::make_shared<State> (*this);}
	virtual bool need2Store () const {return wrappedState->need2Store();}
	auto parent_inserter () {return std::back_inserter(parents);}
	auto begin() {return parents.begin();}
	auto end () {return parents.end ();}
	auto& getWrapped () {return wrappedState;}
      private:
	MiniMC::CPA::State_ptr wrappedState;
	std::vector<std::weak_ptr<State>> parents;
      };

      template<class WrappedQuery>
      struct StateQuery {
	static State_ptr makeInitialState (const MiniMC::Model::Program& prgm) {
	  return std::make_shared<State> (WrappedQuery::makeInitialState (prgm));
	}

	static size_t nbOfProcesses (const State_ptr&  s) {
	  auto ns = std::static_pointer_cast<State> (s);
	  return WrappedQuery::nbOfProcesses(ns->getWrapped ());
	}
	
	static MiniMC::Model::Location_ptr getLocation (const State_ptr& s, proc_id id) {
	  auto ns = std::static_pointer_cast<State> (s);
	  return WrappedQuery::getLocation (ns->getWrapped (),id);
	}
      };

      template<class WrappedTransferer>
      struct Transferer {
	
	static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  
	  auto ns = std::static_pointer_cast<State> (s);
	  auto wrapped = ns->getWrapped();
	  auto wres = WrappedTransferer::doTransfer (wrapped,e,id);
	  if (wres) {
	    auto  res = std::make_shared<State> (wres);
	    res->parent_inserter () = ns;
	    return res;
	  }
	  return nullptr;
	}
      };

      template<class WrappedJoiner>
      struct Joiner {
	static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {
	  auto nl = std::static_pointer_cast<State> (l);
	  auto nr = std::static_pointer_cast<State> (r);

	  auto wres = WrappedJoiner::doJoin (nl->getWrapped(),nr->getWrapped());
	  if (wres) {
	    auto res = std::make_shared<State> (wres);
	    
	    auto inserter = res->parent_inserter ();
	    auto insertFunction = [&] (auto it) {inserter = it;};
	    std::for_each (nl->begin(),nl->end(),insertFunction);
	    std::for_each (nr->begin(),nr->end(),insertFunction);
	    return res;
	  }
	  return nullptr;
	}
	
	
	static bool covers (const State_ptr& l, const State_ptr& r) {
	  auto nl = std::static_pointer_cast<State> (l);
	  auto nr = std::static_pointer_cast<State> (r);
	  return WrappedJoiner::covers (nl,nr);
	}
	
      };
      
      template<class WCPA>
      struct CPADef {
	using Query = StateQuery<typename WCPA::Query>; /**< Class acting a the Query operator*/
	using Transfer = Transferer<typename WCPA::Transfer>; /**< Class acting as the Transfer relation*/
	using Join = Joiner<typename WCPA::Join>; /**< Class acting as Join operation*/
	using Storage = Storer<Join>; /**< This CPAs Storage mechanism*/
	using PreValidate = typename WCPA::PreValidate; /**< The setup needed on Programs to use the CPA*/ 
      };
      

    }
  }
}
