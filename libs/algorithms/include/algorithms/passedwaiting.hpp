#ifndef _PASSED__
#define _PASSED__

#include <vector>
#include <deque>
#include <gsl/pointers>
#include "cpa/interface.hpp"

namespace MiniMC {
  namespace Algorithms {
    class Stack {
    public:
      bool empty () const {
	return thestack.empty();
      }

      void insert (const MiniMC::CPA::State_ptr& state) {
		thestack.push_back (state);
      }
      
      MiniMC::CPA::State_ptr pull () {
		assert(!empty());
		auto  res = thestack.back();
		thestack.pop_back ();
		return res;
      }

	  auto begin () {return thestack.begin();}
	  auto end () {return thestack.begin();}
	  
      auto size () const {return thestack.size();}
      
      
    private:
      std::vector<MiniMC::CPA::State_ptr> thestack;
    };

    class Queue {
    public:
      bool empty () const {
		return thequeue.size();
      }

      void insert (const MiniMC::CPA::State_ptr& state) {
		thequeue.push_front (state);
      }
	  
      MiniMC::CPA::State_ptr pull () {
		assert(!empty());
		auto  res = thequeue.front();
		thequeue.pop_front ();
		return res;
      }

	  auto begin () {return thequeue.begin();}
	  auto end () {return thequeue.begin();}
	  
	  
      auto size () const {return thequeue.size();}
	  
    private:
      std::deque<MiniMC::CPA::State_ptr> thequeue;
    };

	template<class Iterator, class Joiner>
	bool mergeIn (Iterator beg, Iterator end, MiniMC::CPA::State_ptr& state) {
	  bool merged = false;
	  for (; beg != end; ++end) {
		auto res = Joiner::doJoin (*beg,state);
		if (res) {
		  *beg = res;
		  merged = true;
		}
	  }
	  return merged;
	}
	
    template<class StateStorage, class Joiner, class Waiting>
    class PassedWaiting {
    public:
      void insert (gsl::not_null<MiniMC::CPA::State_ptr> ptr) {
		auto insert = [&](auto& inst)->void {
						auto it = waiting.begin();
						auto end = waiting.end ();
						auto state = ptr.get();
						if (!mergeIn<decltype(it),Joiner> (it,end,state))  { 
						  waiting.insert(inst.get());
						  passed++;
						  }
					  } ;
		  if (ptr->need2Store ()) {
			if (!store.isCoveredByStore (ptr.get())) {
			  if (!store.joinState (ptr)) 
				store.saveState (ptr.get(),nullptr);
			  
			  insert(ptr);
			}
		  }
		  else {
			insert (ptr);
		  }
      }
	  
      MiniMC::CPA::State_ptr pull () {
		assert(hasWaiting());
		return waiting.pull ();
      }
	  
      bool hasWaiting () const {
		return !waiting.empty();
      }
	  
      std::size_t getWSize () const {return waiting.size();}
      std::size_t getPSize () const {return passed;}
      
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
