#ifndef _STORAGE__

#include "cpa/interface.hpp"
#include "cpa/state.hpp"


#include <memory>
#include <unordered_set>


namespace MiniMC {
  namespace Storage {
    class Store  {
    public:
      virtual MiniMC::CPA::State_ptr insert (const MiniMC::CPA::State_ptr& s) = 0;
      virtual std::size_t size () const =  0;
      
    };

    class HashStorage : public Store {
    public:
      MiniMC::CPA::State_ptr insert (const MiniMC::CPA::State_ptr& s)  override{
	auto hash = std::hash<MiniMC::CPA::State>{}(*s);
	if (stored.find (hash)==stored.end ()) {
	  
	  stored.insert (hash);
	  return s;
	}
	else {
	  return nullptr;
	}
      }
      std::size_t size () const override {return stored.size ();};
    private:
      std::unordered_set<MiniMC::Hash::hash_t> stored;
    };
    
  }
}


#endif
