/**
 * @file   passedwaiting.hpp
 * @date   Mon Apr 20 17:00:12 2020
 * 
 * @brief  
 * 
 * 
 */
#ifndef _PASSED__
#define _PASSED__

#include "algorithms/successorgen.hpp"
#include "cpa/interface.hpp"
#include "support/feedback.hpp"
#include <functional>
#include <gsl/pointers>
#include <list>

namespace MiniMC {
  namespace Algorithms {

    using FilterFunction = std::function<bool(const MiniMC::CPA::CommonState_ptr&)>;
    using GoalFunction = std::function<bool(const MiniMC::CPA::CommonState_ptr&)>;
    
    struct SimManagerOptions {
      MiniMC::CPA::Storer_ptr storer;
      MiniMC::CPA::Transferer_ptr transfer;
    };

    struct SearchOptions {
      FilterFunction filter = [](const MiniMC::CPA::CommonState_ptr&) { return true; };
      GoalFunction goal = [](const MiniMC::CPA::CommonState_ptr&) { return false; };
    };

    class SimulationManager {
    public:
      SimulationManager(SimManagerOptions opt) : storage(opt.storer),
                                                 generator(opt.transfer) {}
      
      std::size_t getWSize() const { return waiting.size(); }
      std::size_t getPSize() const { return passed; }

      auto stored_begin() { return storage->stored_begin(); }
      auto stored_end() { return storage->stored_end(); }

      auto waiting_begin() { return waiting.begin(); }
      auto waiting_end() { return waiting.end(); }

      void insert(gsl::not_null<MiniMC::CPA::CommonState_ptr> ptr) {
        _insert(ptr);
      }

      MiniMC::CPA::CommonState_ptr step_first(const SearchOptions& sopt) {
        if (waiting.size()) {
          auto search = waiting.front();
          waiting.pop_front();
          return _step(search, sopt);
        }
        return nullptr;
      }

      MiniMC::CPA::CommonState_ptr step_last(const SearchOptions& sopt) {
        if (waiting.size()) {
          auto search = waiting.back();
          waiting.pop_back();
          return _step(search, sopt);
        }
        return nullptr;
      }

      MiniMC::CPA::CommonState_ptr reachabilitySearch(const SearchOptions& sopt) {
        while (waiting.size()) {
          auto res = step_first(sopt);
          if (res) {
            return res;
          }
        }
        return nullptr;
      }

    private:
      MiniMC::CPA::CommonState_ptr _step(MiniMC::CPA::CommonState_ptr ptr, const SearchOptions& soptions) {
        auto succs = generator.generate(ptr);
        for (auto it = succs.first; it != succs.second; ++it) {
          if (soptions.filter(it->state)) {
            if (soptions.goal(it->state)) {
              return it->state;
            } else {
              _insert(it->state);
            }
          }
        }
        return nullptr;
      }

      /** 
       * Insert a state into the waiting list.  If it covered
       * by a State already stored, then it is discarded otherwise it
       * is inserted into the waiting and merged with whichever state
       * it can be merged with in the StateStorage.   
       *
       *
       * @param ptr State to insert
       */
      MiniMC::CPA::CommonState_ptr _insert(MiniMC::CPA::CommonState_ptr ptr) {
        auto insert = [&](const MiniMC::CPA::CommonState_ptr& inst) -> MiniMC::CPA::CommonState_ptr {
	  waiting.push_front(inst);
	  passed++;
	  return inst;
        };

        auto repl_or_insert = [&](const typename MiniMC::CPA::IStorer::JoinPair& p) {
	  auto it = waiting.begin();
	  auto end = waiting.end();
	  auto ff = std::find(it, end, p.orig);
	  if (ff != end)
	    *ff = p.joined;
	  else {
	    waiting.push_front(p.joined);
	    passed++;
	  }
	  return p.joined;
	  
        };

        auto cover = storage->isCoveredByStore(ptr);
	if (cover) {
	  return cover;
	}
	auto join = storage->joinState(ptr);
	if (join.orig) {
	  return repl_or_insert(join);
	}
      
	return insert(ptr);
      }
      
      std::list<MiniMC::CPA::CommonState_ptr> waiting;
      std::size_t passed = 0;
      FilterFunction filter;
      MiniMC::CPA::Storer_ptr storage;
      MiniMC::Algorithms::Generator generator;
    };

  } // namespace Algorithms
} // namespace MiniMC

#endif
