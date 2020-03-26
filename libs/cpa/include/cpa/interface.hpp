#ifndef _CPA_INTERFACE__
#define _CPA_INTERFACE__

#include <ostream>
#include <memory>
#include <unordered_map>
#include "cpa/state.hpp"
#include "support/sequencer.hpp"
#include "support/feedback.hpp"
#include "model/cfg.hpp"
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
    
    struct StateQuery {
      static State_ptr makeInitialState (const MiniMC::Model::Program&) {return std::make_shared<State> ();}
      static size_t nbOfProcesses (const State_ptr& ) {return 0;}
      static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id) {return nullptr;}
	};
    
    struct Transferer {
      static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr&,proc_id) {return nullptr;}
    };
	
    struct Joiner {  
	  static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return nullptr;}
	  //Returns true if l covers r
	  static bool covers (const State_ptr& l, const State_ptr& r) { return false;}
    };

    struct PrevalidateSetup {
      static void setup (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
      static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
	};

	template<class JoinOperation>
    class Storer {
    public:
      using StorageTag = MiniMC::Hash::hash_t;
	  virtual ~Storer () {}
      bool saveState (const State_ptr& state,StorageTag* tag = nullptr) {
		if (tag)
		  *tag = actualStore.size();
		actualStore.emplace_back(state);		     
		 
		return true;
	  }
	  
      State_ptr loadState (StorageTag st) {
		return actualStore.at(st);
	  }
	  
	  std::size_t joinState (const State_ptr& state) {
		std::size_t merged = 0;
		for (auto& it : actualStore) {
		  auto res = JoinOperation::doJoin (it,state);
		  if (res) {
			it = res;
			merged++;
		  }
		  
		}
		return merged;
	  }

	  bool isCoveredByStore (const State_ptr& state) {
		for (auto& it : actualStore) {
		  if (JoinOperation::covers (it,state))
			return true;
		}
		return false;
	  }
	  
    private:
      std::vector<State_ptr> actualStore;
    };
	
    struct CPADef {
      using Query = StateQuery;
      using Transfer = Transferer;
      using Join = Joiner;
      using Storage = Storer<Join>; 
	  using PreValidate = PrevalidateSetup;
	};
    
  }
}

#endif
