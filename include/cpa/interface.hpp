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

#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include <memory>
#include <ostream>
#include <unordered_map>

namespace MiniMC {
  namespace CPA {

    struct InitialiseDescr {
    public:
      InitialiseDescr (std::vector<MiniMC::Model::Function_ptr> entries,
		       MiniMC::Model::HeapLayout heap) : entries (std::move(entries)),
							  heap (std::move(heap)) {}

      auto& getEntries () const {return entries;}
      auto& getHeap () const {return heap;}
      
      
    private:
      std::vector<MiniMC::Model::Function_ptr> entries;
      MiniMC::Model::HeapLayout heap;
    };
    
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
      virtual State_ptr makeInitialState(const InitialiseDescr& ) = 0;
    };
    
    using StateQuery_ptr = std::shared_ptr<StateQuery>;

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
      virtual State_ptr doTransfer(const State_ptr&, const MiniMC::Model::Edge_ptr&, proc_id) { return nullptr; }
    };

    using Transferer_ptr = std::shared_ptr<Transferer>;

    struct Joiner {
      /** 
       * Join two  states \p l and \p r with each other
       *
       * @return  the joined state of nullptr if the states cannot be
       * merged. 
       */
      virtual State_ptr doJoin(const State_ptr&, const State_ptr&) { return nullptr; }

      /** 
       * Test if \p l covers \p r i.e. whether the behaviour of \l
       * includes that of \p r
       */
      virtual bool covers(const State_ptr&, const State_ptr&) {
        return false;
      }
    };

    using Joiner_ptr = std::shared_ptr<Joiner>;

    
    class IStorer {
    public:
      using Iterator = std::vector<MiniMC::CPA::State_ptr>::iterator;
      using StorageTag = MiniMC::Hash::hash_t;

      virtual ~IStorer() {}
      virtual bool saveState(const State_ptr& state, StorageTag* tag = nullptr) = 0;
      virtual State_ptr loadState(StorageTag st) = 0;
      struct JoinPair {
        State_ptr orig;
        State_ptr joined;
      };
      virtual IStorer::JoinPair joinState(const State_ptr& state) = 0;
      virtual State_ptr isCoveredByStore(const State_ptr& state) = 0;
      //THese breeak the interfacec
      virtual Iterator stored_begin() = 0;
      virtual Iterator stored_end() = 0;
    };

    using Storer_ptr = std::shared_ptr<IStorer>;

    class Storer : public IStorer {
    public:
      Storer(const Joiner_ptr& join) : JoinOperation(join) {}
      virtual ~Storer() {}

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
      bool saveState(const State_ptr& state, StorageTag* tag = nullptr) {
        assert(!isCoveredByStore(state));
        if (tag)
          *tag = actualStore.size();
        actualStore.emplace_back(state);

        return true;
      }

      State_ptr loadState(StorageTag st) {
        return actualStore.at(st);
      }

      /** 
       * Try to join \p state into a state states already stored
       *
       * @param state the State we try to join with
       *
       * @return Merged State or nullptr if unsuccessful
       */
      IStorer::JoinPair joinState(const State_ptr& state) {
        for (auto& it : actualStore) {
          auto res = JoinOperation->doJoin(it, state);
          if (res) {
            auto orig = it;
            it = res;
            return {.orig = orig, .joined = res};
          }
        }
        saveState(state);
        return {.orig = nullptr, .joined = state};
      }

      /** 
       * Check whether a state is covered by some state already in
       * this storage.
       *
       * @param state the state to check for 
       *
       * @return state covering state
       */
      State_ptr isCoveredByStore(const State_ptr& state) {
        for (auto& it : actualStore) {
          if (JoinOperation->covers(it, state)) {
            return it;
          }
        }
        return nullptr;
      }

      IStorer::Iterator stored_begin() { return actualStore.begin(); }
      IStorer::Iterator stored_end() { return actualStore.end(); }

    private:
      std::vector<State_ptr> actualStore;
      Joiner_ptr JoinOperation;
    };

    struct ICPA {
      virtual ~ICPA() {}
      virtual StateQuery_ptr makeQuery() const = 0;
      virtual Transferer_ptr makeTransfer() const = 0;
      virtual Joiner_ptr makeJoin() const = 0;
      virtual Storer_ptr makeStore() const = 0;
    };

    using CPA_ptr = std::shared_ptr<ICPA>;

    template <
        class Query,
        class Transfer,
        class Joiner,
        class Store>
    struct CPADef : public ICPA {
      virtual StateQuery_ptr makeQuery() const { return std::make_shared<Query>(); }
      virtual Transferer_ptr makeTransfer() const { return std::make_shared<Transfer>(); }
      virtual Joiner_ptr makeJoin() const { return std::make_shared<Joiner>(); }
      virtual Storer_ptr makeStore() const { return std::make_shared<Store>(std::make_shared<Joiner>()); }
    };
    
  } // namespace CPA
} // namespace MiniMC

#endif
