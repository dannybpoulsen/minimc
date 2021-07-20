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

#include <functional>
#include <gsl/pointers>
#include "cpa/interface.hpp"
#include "support/queue_stack.hpp"

namespace MiniMC {
  namespace Algorithms {
    using Stack = MiniMC::Support::Stack<MiniMC::CPA::State>;
    using Queue = MiniMC::Support::Queue<MiniMC::CPA::State>;    

    using FilterFunction =   std::function<bool(const MiniMC::CPA::State_ptr&)>; 
    using DelaySearchPredicate =   std::function<bool(const MiniMC::CPA::State_ptr&)>; 
    using StoreStatePredicate =   std::function<bool(const MiniMC::CPA::State_ptr&)>; 
	
	
    struct PWOptions {
      FilterFunction filter = [](const MiniMC::CPA::State_ptr& s) {return true;};
      DelaySearchPredicate delay = [](const MiniMC::CPA::State_ptr& s) {return !s->ready2explore ();};
      StoreStatePredicate storage = [](const MiniMC::CPA::State_ptr& s) {return s->need2Store ();};
      MiniMC::CPA::Storer_ptr storer;
      MiniMC::CPA::Joiner_ptr joiner;
    };
    
	
    /** 
     * PassedWaiting list implemented using the \p StateStorage operations and \p Joiner operations.
     * The SearchStrategy is given by \p Waiting  which shoukd be either Stack or Queue. 
     *  
     */
    template<class Waiting>
    class PassedWaiting {
    public:

	  
      PassedWaiting (const PWOptions& opt) : filter(opt.filter),
					     delay(opt.delay),
					     doStore (opt.storage),
					     storage(opt.storer),
					     joiner(opt.joiner)
      {}
      /** 
       * Insert a state into the passed waiting list.  If it covered
       * by a State already stored, then it is discarded otherwise it
       * is inserted into the waiting and merged with whichever state
       * it can merged with in the StateStorage.   
       *
       *
       * @param ptr State to insert
       */
      MiniMC::CPA::State_ptr insert (gsl::not_null<MiniMC::CPA::State_ptr> ptr) {
	if (!filter(ptr))
	  return nullptr;
	auto insert = [&](const MiniMC::CPA::State_ptr& inst) -> MiniMC::CPA::State_ptr {
	  if (delay(inst))
	    return nullptr;
	  else {
	    waiting.insert(inst);
	    passed++;
	    return inst;
	  }
	};
	auto repl_or_insert = [&](const typename MiniMC::CPA::IStorer::JoinPair& p) {
	  if (!delay(p.orig)) {
	    auto it = waiting.begin();
	    auto end = waiting.end ();
	    auto ff = std::find (it,end,p.orig);
	    if (ff != end) 
	      *ff = p.joined;
	    else {
	      waiting.insert (p.joined);
	      passed++;
			  
	    }
	    return p.joined;
			  
	  }
	  else {
	    return insert (p.joined);
	  }
	};
		  
	if (doStore(ptr)) {
	  auto cover = storage->isCoveredByStore (ptr.get());
	  if (cover) {
	    return cover;
	  }
	  auto join = storage->joinState (ptr.get()); 
	  if (join.orig) {
	    return repl_or_insert (join);
	  }
	}
	return insert (ptr);
      }
	  
      /** Extract the first State*/ 
      MiniMC::CPA::State_ptr pull () {
	assert(hasWaiting());
	return waiting.pull ();
      }
	  
      /** 
	   
       * @return true if it has states waiting, false otherwise
       */
      bool hasWaiting () const {
	return !waiting.empty();
      }
	  
      std::size_t getWSize () const {return waiting.size();}
      std::size_t getPSize () const {return passed;}

      auto stored_begin () {return storage->stored_begin();}
      auto stored_end () {return storage->stored_end();}
      
      
    private:
      Waiting waiting;
      std::size_t passed = 0;
      FilterFunction filter;
      DelaySearchPredicate delay;
      StoreStatePredicate doStore;  
      MiniMC::CPA::Storer_ptr storage;
      MiniMC::CPA::Joiner_ptr joiner;
      
    };	
	
    using DFSWaiting = PassedWaiting<Stack>;

    using BFSWaiting = PassedWaiting<Queue>;
    
    
  }
}


#endif
