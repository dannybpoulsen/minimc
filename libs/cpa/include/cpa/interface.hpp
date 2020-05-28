/**
 * @file   interface.hpp
 * @date   Mon Apr 20 17:18:43 2020
 * 
 * MiniMC uses something called CPAs to encapsulate different
 * "verification views" on  a program. For instance one CPA may
 * "track" the program locations of all processes/threads while
 * another tracks their variable values and memory. Combining these
 * two together provides an explicit verification context 
 *
 * 
 */
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

	/** 
	 * The StateQuery "interface" is a CPAs way of creating the
	 * initial state of a program and for algorithms to query a State
	 * about the number of processes and the location of each process
	 */
	struct StateQuery {
	  /** 
	   * 
	   * Create a new initial state for the program
	   *
	   * @return the create State
	   */
	  static State_ptr makeInitialState (const MiniMC::Model::Program&) {return std::make_shared<State> ();}

	  /** 
	   * Query the given state how many processes it has
	   *
	   * @return number of processes
	   */
	  static size_t nbOfProcesses (const State_ptr& ) {return 0;}

	  /** 
	   * Get the current Location of process \p id 
	   * 
	   *
	   * @return the Location of \p id or nullptr if there no process
	   * \p id
	   */
	  static MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id id) {return nullptr;}
	};

	/** 
	 * The Tranferer generates successor for States
	 */ 
    struct Transferer {
	  /** 
	   * Comput the successor state of \p s by performing edge \p e for
	   * process \p 
	   *
	   * @param s 
	   *
	   * @return the resulting State of nullptr if the edge cannot be
	   * performed (which may happen for instance) when a guard is
	   * false) 
	   */
      static State_ptr doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {return nullptr;}
    };

	
    struct Joiner {  
	  /** 
	   * Join two  states \p l and \p r with each other
	   *
	   * @return  the joined state of nullptr if the states cannot be
	   * merged. 
	   */
	  static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {return nullptr;}

	  /** 
	   * Test if \p l covers \p r i.e. whether the behaviour of \l
	   * includes that of \p r
	   */
	  static bool covers (const State_ptr& l, const State_ptr& r) { return false;}
    };

    struct PrevalidateSetup {
	  /** 
	   * Add modification needed to use the associated  CPA to \p seq 
	   *
	   */
	  static void setup (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
	  /** 
	   * Add checks to \p seq  that guarantees this CPA can be run
	   * without encountering runtime errors. 
	   */
      static void validate (MiniMC::Support::Sequencer<MiniMC::Model::Program>& seq, MiniMC::Support::Messager& mess) {}
	};

	
	template<class JoinOperation>
    class Storer {
    public:
      using StorageTag = MiniMC::Hash::hash_t;
	  virtual ~Storer () {}

	  /** 
	   * Insert a State into the store. It must be the case that
	   * isCoveredByStore(\pstate) is false.
	   *
	   * @param state State to insert
	   * @param[out] tag will be set to a tag that can be used to load
	   * the state (see \ref loadState) 
	   *
	   * @return 
	   */
      bool saveState (const State_ptr& state,StorageTag* tag = nullptr) {
		assert(!isCoveredByStore (state));
		if (tag)
		  *tag = actualStore.size();
		actualStore.emplace_back(state);		     
		 
		return true;
	  }

	  
      State_ptr loadState (StorageTag st) {
		return actualStore.at(st);
	  }

	  /** 
	   * Try to join \p state into with states already stored
	   *
	   * @param state the State we try to join with
	   *
	   * @return number of states we managed to merge with
	   */
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

	  /** 
	   * Check whether a state is covered by some state already in
	   * this storage.
	   *
	   * @param state the state to check for 
	   *
	   * @return true if it covered, false otherwise
	   */
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

	/** All CPAs are defined using a struct like this */
    struct CPADef {
      using Query = StateQuery; /**< Class acting a the Query operator*/
      using Transfer = Transferer; /**< Class acting as the Transfer relation*/
      using Join = Joiner; /**< Class acting as Join operation*/
      using Storage = Storer<Join>; /**< This CPAs Storage mechanism*/
	  using PreValidate = PrevalidateSetup; /**< The setup needed on Programs to use the CPA*/ 
	};
    
  }
}

#endif
