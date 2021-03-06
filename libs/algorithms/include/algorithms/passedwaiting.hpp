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

#include <gsl/pointers>
#include "cpa/interface.hpp"
#include "support/queue_stack.hpp"

namespace MiniMC {
  namespace Algorithms {
	using Stack = MiniMC::Support::Stack<MiniMC::CPA::State>;
	using Queue = MiniMC::Support::Queue<MiniMC::CPA::State>;
	
    /*template<class Iterator, class Joiner>
    State_ptr mergeIn (Iterator beg, Iterator end, MiniMC::CPA::State_ptr& state) {
      bool merged = false;
      for (; beg != end; ++beg) {
	auto res = Joiner::doJoin (*beg,state);
	if (res) {
	  *beg = res;
	  return res;
	}
      }
      return nullptr;
    }
    */
    
	
    /** 
     * PassedWaiting list implemented using the \p StateStorage operations and \p Joiner operations.
     * The SearchStrategy is given by \p Waiting  which shoukd be either Stack or Queue. 
     *  
     */
    template<class StateStorage, class Joiner, class Waiting>
    class PassedWaiting {
    public:
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
	auto insert = [&](auto& inst) {
			/*auto it = waiting.begin();
			auto end = waiting.end ();
			auto state = ptr.get();
			auto res = mergeIn<decltype(it),Joiner> (it,end,state) 
			  if (res)
			    return res;
			*/
			waiting.insert(inst.get());
			passed++;
			return inst.get();	
		      };
	auto repl_or_insert = [&](const typename StateStorage::JoinPair& p) {
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
			      };
	
	if (ptr->need2Store ()) {
	  auto cover = store.isCoveredByStore (ptr.get());
	  if (cover) {
		Joiner::coverCopy (ptr.get(),cover);
	    return cover;
	  }
	  auto join = store.joinState (ptr.get()); 
	  if (join.orig) {
	    return repl_or_insert (join);
	    //return join.joined;
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

      auto stored_begin () {return store.stored_begin();}
      auto stored_end () {return store.stored_end();}
      
      
    private:
      StateStorage store;
      Waiting waiting;
      std::size_t passed = 0;
    };	
   
    template<class StateStorage,class Joiner>
    using DFSBaseWaiting = PassedWaiting<StateStorage,Joiner,Stack>;

    template<class StateStorage,class Joiner>
    using BFSBaseWaiting = PassedWaiting<StateStorage,Joiner,Queue>;

    template<class CPA>
    using CPADFSPassedWaiting = DFSBaseWaiting<typename CPA::Storage, typename CPA::Join>;

    template<class CPA>
    using CPABFSPassedWaiting = BFSBaseWaiting<typename CPA::Storage, typename CPA::Join>;
	
  }
}


#endif
