#ifndef _STATE__
#define _STATE__

#include <ostream>
#include <memory>
#include "model/cfg.hpp"
#include "hash/hashing"

namespace MiniMC {
  namespace CPA {
    class State {
    public:
      ~State () {}
      virtual std::ostream& output (std::ostream& os) const {os << "_";}
      virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {return 0;}
      virtual std::shared_ptr<State> copy () const {return std::make_shared<State> ();}
    };
    
    using State_ptr = std::shared_ptr<State>;
    
    inline std::ostream& operator<< (std::ostream& os, const State& state) {
      return state.output(os);
    }

    struct StateQuery {
      static State_ptr makeInitialState (const MiniMC::Model::Program&) {return std::make_shared<State> ();}
      static size_t nbOfProcesse (const State_ptr& ) {return 0;}
      static MiniMC::Model::Location_ptr getLocation (const State_ptr&, size_t) {return nullptr;}
    };
    
    struct Transferer {
      static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,size_t ) {return nullptr;}
    };

    struct Joiner {  
      static State_ptr doJoin (const State_ptr& l, const state_ptr& r) {return r;}
    };
    
    class Storer {
    public:
      StorageTag = std::size_t;
      ~Storage () {}
      StorageTag saveState (const State_pt& state) {return 0;}
      State_ptr loadState (StorageTag) {return std::make_shared<State> ();}
    };

    struct CPADef {
      using Query = StateQuery;
      using Transfer = Transferer;
      using Joing = Joiner;
      using Storage = Storer; 
    };
    
  }
}

#endif
