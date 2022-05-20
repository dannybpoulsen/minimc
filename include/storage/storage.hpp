#ifndef _STORAGE__

#include "cpa/interface.hpp"
#include "cpa/state.hpp"


#include <memory>
#include <unordered_set>


namespace MiniMC {
  namespace Storage {
    class Store  {
    public:
      virtual bool insert (const MiniMC::CPA::AnalysisState& s) = 0;
      virtual std::size_t size () const =  0;
      
    };

    class HashStorage : public Store {
    public:
      bool insert (const MiniMC::CPA::AnalysisState& s)  override{
	auto hash = std::hash<MiniMC::CPA::AnalysisState>{}(s);
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
