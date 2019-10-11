#include "model/cfg.hpp"
#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "cpa/location.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {
      class State : public MiniMC::CPA::State  {
      public:
	State (const std::vector<MiniMC::Model::Location*>& locations ) : locations(locations) {
	  
	}
	virtual std::ostream& output (std::ostream& os) const {
	  os << "[ ";
	  for (auto l : locations) {
	    assert(l);
	    os << l->getName() << ", ";
	  }
	  return os << "]";
	}
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
	  return MiniMC::Hash::Hash<MiniMC::Model::Location*> (locations.data(),locations.size(),seed);
	}
	virtual std::shared_ptr<MiniMC::CPA::Location::State> lcopy () const {return std::make_shared<State> (locations);}
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return lcopy();}

	
	size_t nbOfProcesses () const {return locations.size();}
	MiniMC::Model::Location_ptr getLocation (size_t i) const  {return locations[i]->shared_from_this();}
	void setLocation (size_t i,MiniMC::Model::Location* l)   {locations[i] = l;}
      private:
	std::vector<MiniMC::Model::Location*> locations;
      };

      MiniMC::CPA::State_ptr MiniMC::CPA::Location::Transferer::doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& edge,proc_id id) {
	auto state = static_cast<State*> (s.get ());
	assert(id < state->nbOfProcesses());
	if (edge->getFrom().get() == state->getLocation (id)) {
	  auto nstate = state->lcopy ();
	  nstate->setLocation (id,edge->getTo().get().get());
	  
	  return nstate;
	}
	return nullptr;
      }
      
      State_ptr MiniMC::CPA::Location::StateQuery::makeInitialState (const MiniMC::Model::Program& p) {
	std::vector<MiniMC::Model::Location*> locs;
	for (auto& f : p.getEntryPoints()) {
	  locs.push_back(f->getCFG()->getInitialLocation().get().get());
	}
	return std::make_shared<State> (locs);
      }
      
      size_t MiniMC::CPA::Location::StateQuery::nbOfProcesses (const State_ptr& s) {
	auto state = static_cast<const State*> (s.get ());
	return state->nbOfProcesses ();
      }

      MiniMC::Model::Location_ptr MiniMC::CPA::Location::StateQuery::getLocation (const State_ptr& s, proc_id id) {
	auto state = static_cast<const State*> (s.get ());
	return state->getLocation (id);
      }
      
      
    }
  }
}
