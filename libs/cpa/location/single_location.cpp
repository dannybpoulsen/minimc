#include "model/cfg.hpp"
#include "hash/hashing.hpp"
#include "cpa/interface.hpp"
#include "cpa/location.hpp"
#include "support/pointer.hpp"


namespace MiniMC {
  namespace CPA {
	namespace SingleLocation {
	  class State : public MiniMC::CPA::State  {
      public:
		State (const MiniMC::Model::Location_ptr& loc ) : location(loc.get()) {
		  ready = !location->getInfo().template is<MiniMC::Model::Attributes::ConvergencePoint> ();
		}
		
		State (const State& ) = default;
		
		virtual std::ostream& output (std::ostream& os) const {
		  return os << "[" << location->getInfo () <<"]" ;
		}
		virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const override {
		  MiniMC::Hash::hash_t s = seed;
		  
		  MiniMC::Hash::hash_combine(s,location);
		  MiniMC::Hash::hash_combine(s,ready);
		  return s;
		  
		}
		virtual std::shared_ptr<MiniMC::CPA::SingleLocation::State> lcopy () const {return std::make_shared<State> (*this);}
		virtual std::shared_ptr<MiniMC::CPA::State> copy () const {return lcopy();}
		
		
		size_t nbOfProcesses () const override {return 1;}
		MiniMC::Model::Location_ptr getLocation (proc_id id ) const  override  {assert(id ==0); return location->shared_from_this();}
		void setLocation (MiniMC::Model::Location* l)   {
		  location = l;
		  ready = !location->getInfo().template is<MiniMC::Model::Attributes::ConvergencePoint> ();
		}
		bool need2Store () const {
		  return location->getInfo().template is<MiniMC::Model::Attributes::NeededStore> ();
			
		}

		virtual bool assertViolated () const {
		  return location->getInfo().template is<MiniMC::Model::Attributes::AssertViolated> ();
		  
		}
		
		
		
		bool ready2explore () const override {return ready;}
		
		std::shared_ptr<State> join (const State& oth) const {
		  if (location == oth.location) {
			auto nstate = std::make_shared<State> (location->shared_from_this());
			nstate->ready = true;
			return nstate;
		  }
		  return nullptr;
		}
		
	  private:
		MiniMC::Model::Location* location;
		bool ready;
		
	  };
	  
      MiniMC::CPA::State_ptr Transferer::doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& edge,proc_id id) {
		auto state = static_cast<const MiniMC::CPA::SingleLocation::State*> (s.get ());
		assert(id < state->nbOfProcesses());
		if (edge->getFrom().get() == state->getLocation (0)) {
		  auto nstate = state->lcopy ();
		  nstate->setLocation (edge->getTo().get().get());
		  
		  if (edge->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
			auto& inst =  edge->getAttribute<MiniMC::Model::AttributeType::Instructions> ().last(); 
			if (inst.getOpcode () == MiniMC::Model::InstructionCode::Call) {
			  //throw NotImplemented<MiniMC::Model::InstructionCode::Call> ();
			  
			}
			

			else if (MiniMC::Model::isOneOf<MiniMC::Model::InstructionCode::RetVoid,
					 MiniMC::Model::InstructionCode::Ret> (inst)) {
			  //throw NotImplemented<MiniMC::Model::InstructionCode::RetVoid> ();
			  
			}
			
		  }
		  return nstate;
		}
			
		
		return nullptr;
		
	  }
		
	
		
	  
      State_ptr StateQuery::makeInitialState (const MiniMC::Model::Program& p) {
		std::vector<MiniMC::Model::Location_ptr> locs;
		for (auto& f : p.getEntryPoints()) {
		  locs.push_back(f->getCFG()->getInitialLocation().get());
		}
		assert(locs.size()==1);
		return std::make_shared<State> (locs[0]);
      }
      
	  State_ptr Joiner::doJoin (const State_ptr& l, const State_ptr& r) {
		auto lstate = std::static_pointer_cast<const State> (l);
		auto rstate = std::static_pointer_cast<const State> (r);
		return lstate->join (*rstate);
	  }
	  
	  
    }
  }
}
