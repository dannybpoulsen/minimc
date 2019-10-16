#include "model/cfg.hpp"
#include "hash/hashing.hpp"
#include "cpa/interface.hpp"


namespace MiniMC {
  namespace CPA {
    bool MiniMC::CPA::Storer::saveState (const State_ptr& state, MiniMC::CPA::Storer::StorageTag* tag) {
	auto hash = state->hash ();
	if (!actualStore.count(hash)) {
	  actualStore.insert(std::make_pair (hash,state));		     
	  if (tag)
	    *tag = hash;
	  return true;
	}
	return false;
      }

      MiniMC::CPA::State_ptr MiniMC::CPA::Storer::loadState (StorageTag st) {
	return actualStore.at(st);
      }
  }
}
