#ifndef _STATE__
#define _STATE__

#include <ostream>
#include <memory>
#include <unordered_map>
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "model/cfg.hpp"
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
    using proc_id = std::size_t;
    class State {
    public:
      ~State () {}
      virtual std::ostream& output (std::ostream& os) const {return os << "_";}
      virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {return 0;}
      virtual std::shared_ptr<State> copy () const {return std::make_shared<State> ();}
      virtual bool isPotentialLoop () const {return false;}
    };
    
    using State_ptr = std::shared_ptr<State>;
    
    inline std::ostream& operator<< (std::ostream& os, const State& state) {
      return state.output(os);
    }

    struct StateQuery {
      static State_ptr makeInitialState (const MiniMC::Model::Program&) {return std::make_shared<State> ();}
      static size_t nbOfProcesses (const State_ptr& ) {return 0;}
      static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id) {return nullptr;}
    };
    
    struct Transferer {
      static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id) {return nullptr;}
    };

    struct Joiner {  
      static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return r;}
    };

    struct PrevalidateSetup {
      static void setup (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
      static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
	};
	
    class Storer {
    public:
      using StorageTag = MiniMC::Hash::hash_t;
      virtual ~Storer () {}
      bool saveState (const State_ptr& state,StorageTag* tag = nullptr);
      State_ptr loadState (StorageTag);
    private:
      std::unordered_map<MiniMC::Hash::hash_t, State_ptr> actualStore;;
    };
	
    struct CPADef {
      using Query = StateQuery;
      using Transfer = Transferer;
      using Join = Joiner;
      using Storage = Storer; 
	  using PreValidate = PrevalidateSetup;
	};
    
  }
}

#endif
