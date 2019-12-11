#ifndef _PASSED__
#define _PASSED__

#include <stack>
#include <queue>
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
	thestack.push (state);
      }
      
      MiniMC::CPA::State_ptr pull () {
	assert(!empty());
	auto  res = thestack.top();
	thestack.pop ();
	return res;
      }

      auto size () const {return thestack.size();}
      
      
    private:
      std::stack<MiniMC::CPA::State_ptr> thestack;
    };

    class Queue {
    public:
      bool empty () const {
	return thequeue.empty();
      }

      void insert (const MiniMC::CPA::State_ptr& state) {
	thequeue.push (state);
      }

      MiniMC::CPA::State_ptr pull () {
	assert(!empty());
	auto  res = thequeue.front();
	thequeue.pop ();
	return res;
      }

      auto size () const {return thequeue.size();}
      
    private:
      std::queue<MiniMC::CPA::State_ptr> thequeue;
    };

    template<class StateStorage, class Waiting>
    class PassedWaiting {
    public:
      void insert (gsl::not_null<MiniMC::CPA::State_ptr> ptr) {
	if (store.saveState (ptr.get(),nullptr)) {
	  passed++;
	  waiting.insert (ptr.get());
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

    template<class StateStorage>
    using DFSBaseWaiting = PassedWaiting<StateStorage,Stack>;

    template<class StateStorage>
    using BFSBaseWaiting = PassedWaiting<StateStorage,Queue>;

    template<class CPA>
    using CPADFSPassedWaiting = DFSBaseWaiting<typename CPA::Storage>;

    template<class CPA>
    using CPABFSPassedWaiting = BFSBaseWaiting<typename CPA::Storage>;

  }
}


#endif
