#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/location.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/vm/vmt.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Location {

      struct LocationState {
        void push(MiniMC::Model::Location* l) {
	  stack.push_back(l);
        }

        void pop() {
	  if (stack.size() >= 1)
            stack.pop_back();
        }


	void setLocation (MiniMC::Model::Location* l) {
	  stack.back () = l;
	}
	
	
        auto& cur() const {
          assert(stack.size());
          return stack.back();
        }

	bool active () const {
	  return stack.size();
	}
	
        virtual MiniMC::Hash::hash_t hash() const {
	  MiniMC::Hash::Hasher hash;
	  for (auto& t : stack)
	    hash << *t;
	  return hash;
	}
	
        std::vector<MiniMC::Model::Location*> stack;
      };
    } // namespace Location
  }   // namespace CPA
} // namespace MiniMC

namespace MiniMC {
  namespace CPA {
    namespace Location {
      class State : public MiniMC::CPA::CFAState,
		    private MiniMC::CPA::LocationInfo
      {
      public:
        State(const std::vector<LocationState>& locations) : locations(locations) {
        }

        State(const State&) = default;
	
	
        virtual MiniMC::Hash::hash_t hash() const override {
	  MiniMC::Hash::Hasher hash;
	  for (auto& t : locations)
            hash << t;
	  return hash;
	}

	virtual std::shared_ptr<MiniMC::CPA::Location::State> lcopy() const { return std::make_shared<State>(*this); }
        virtual std::shared_ptr<MiniMC::CPA::CFAState> copy() const override { return lcopy(); }
	
        size_t nbOfProcesses() const override { return locations.size(); }
        MiniMC::Model::Location& getLocation(size_t i) const override { return *locations.at(i).cur(); }
	bool isActive(size_t i) const override  { return locations.at(i).active(); }
	void setLocation(size_t i, MiniMC::Model::Location* l) { 
	  locations[i].setLocation(l);
        }
        void pushLocation(size_t i, MiniMC::Model::Location* l) { locations[i].push(l); }
        void popLocation(size_t i) { locations[i].pop(); }
        
	virtual const MiniMC::CPA::LocationInfo& getLocationState () const {return *this;}
	
	
      private:
        std::vector<LocationState> locations;
      };

      class DummyOperations {};
      class DummyCaster {};
      
      
      struct VMState : public MiniMC::VMT::SimpStackControl{
	using Domain = int;
	VMState (State& s, std::size_t id) : state(s), id(id) {
	  
	}

	void  push (MiniMC::Model::Location_ptr loc ,std::size_t,  const MiniMC::Model::Value_ptr& ) {
	  state.pushLocation(id, loc.get());
	}
	
	void popNoReturn () {
	  state.popLocation(id);
	}

	auto& getStackControl () {return *this;}
	
      private:
	State& state;
	std::size_t id;
      };
	
      MiniMC::CPA::State_ptr<CFAState> MiniMC::CPA::Location::Transferer::doTransfer(const CFAState& s, const Transition& transition ) {
	const MiniMC::Model::Edge& edge = *transition.edge;
	proc_id id = transition.proc;
        auto& state = static_cast<const State&>(s);
        assert(id < state.nbOfProcesses());
	
	if (edge.getFrom ().get () != &state.getLocation(id))
	  return nullptr;
	
	auto nstate = state.lcopy();
	nstate->setLocation(id, edge.getTo().get());
	if (!edge.getInstructions ()) {
	  return nstate;
	}
	
	MiniMC::VMT::Engine<DummyOperations> engine {DummyOperations{},prgm};
        VMState vmstate{*nstate, id};
	engine.execute (edge.getInstructions (),vmstate);
        return nstate;
	
      }
      
    

      State_ptr<CFAState> MiniMC::CPA::Location::CPA::makeInitialState(const InitialiseDescr& p) {
        std::vector<LocationState> locs;
        for (auto& f : p.getEntries()) {
          locs.emplace_back();
          locs.back().push(f.getFunction()->getCFA().getInitialLocation().get());
        }
        return std::make_shared<State>(locs);
      }

      
    } // namespace Location
  }   // namespace CPA
} // namespace MiniMC
