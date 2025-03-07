#ifndef _STORAGE__

#include "minimc/cpa/interface.hpp"
#include "minimc/cpa/state.hpp"


#include <memory>
#include <unordered_set>


namespace MiniMC {
  namespace Storage {
    template<class State>
    class Store  {
    public:
      virtual ~Store () {}
      virtual bool insert (const State& s) = 0;
      virtual std::size_t size () const =  0;
      
    };

    template<class State>
    class HashStorage : public Store<State> {
    public:
      bool insert (const State& s)  override{
	auto hash = std::hash<State>{}(s);
	if (stored.find (hash)==stored.end ()) {
	  
	  stored.insert (hash);
	  return true;
	}
	else {
	  return false;
	}
      }
      std::size_t size () const override {return stored.size ();};
    private:
      std::unordered_set<MiniMC::Hash::hash_t> stored;
    };


    
  }
}


#endif
