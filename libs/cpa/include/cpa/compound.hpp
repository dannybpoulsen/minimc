#ifndef _COMPOUND__
#define _COMPOUND__

#include <initializer_list>
#include <vector>
#include <set>
#include "cpa/interface.hpp"
#include "hash/hashing.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Compounds {
      template<size_t S>
      class State : public MiniMC::CPA::State {
      public:
	State (std::initializer_list<MiniMC::CPA::State_ptr> l) {
	  assert (l.size () == S);
	  std::copy(l.begin(), l.end(), states.begin());
	}

	State (std::vector<MiniMC::CPA::State_ptr>& l) {
	  assert (l.size () == S);
	  std::copy(l.begin(), l.end(), states.begin());
	}

	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const  {
	  MiniMC::Hash::hash_t hash = seed;
	  for (auto& state : states) {
	    hash = state->hash(hash);
	  }
	  return hash;
	}

	virtual std::ostream& output (std::ostream& os) const  {
	  for (auto& state : states) {
	    state->output (os);
	    os << "\n________________\n";
	  }
	  return os;
	}

	template<size_t i>
	const State_ptr& get () const  {return states[i];}
	
      private:
	std::array<MiniMC::CPA::State_ptr,S> states;
      };

      template<unsigned int N, class Head, class... Tail>
      struct GetNthTemplateArgument : GetNthTemplateArgument<N-1,Tail...>
      {
      };


      template<class Head, class... Tail>
      struct GetNthTemplateArgument<0,Head,Tail...>
      {
	using Temp = Head;;
      };
      
      template<size_t ask, class ... args>
      struct StateQuery {
	static State_ptr makeInitialState (const MiniMC::Model::Program& prgm) {
	  std::initializer_list<MiniMC::CPA::State_ptr> init ( {(args::Query::makeInitialState (prgm)) ...});
	  return std::make_shared<State<sizeof... (args)>> (init);
	}						       

	static size_t nbOfProcesses (const State_ptr& a) {
	  auto s = static_cast<State<sizeof... (args)>&> (*a);
	  return GetNthTemplateArgument<ask,args...>::Temp::Query::nbOfProcesses (s.template get<ask> ());
	}

	
	static MiniMC::Model::Location_ptr getLocation (const State_ptr& a, proc_id id) {
	  auto s = static_cast<State<sizeof... (args)>&> (*a);
	  return GetNthTemplateArgument<ask,args...>::Temp::Query::getLocation (s.template get<ask> (),id);
	  
	}
      };
      
      template<size_t i, size_t statesize, class A, class ... args>
      struct BuildVector {
	static void doIt (std::vector<MiniMC::CPA::State_ptr>& vec,const State<statesize>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  vec.push_back (A::Transfer::doTransfer (s.template get<i> (),e,id));
	  BuildVector<i+1,statesize,args...>::doIt (vec,s,e,id);
	}
      };

      template<size_t i, size_t statesize, class A>
      struct BuildVector<i,statesize,A> {
	
	static void doIt (std::vector<MiniMC::CPA::State_ptr>& vec,const State<statesize>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  vec.push_back (A::Transfer::doTransfer (s.template get<i> (),e,id));
	}
      };
      
      
      template<class ... args>
      struct Transferer {
	static State_ptr doTransfer (const State_ptr& a, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	  auto s = static_cast<State<sizeof... (args)>&> (*a);
	  std::vector<MiniMC::CPA::State_ptr> vec;
	  BuildVector<0,sizeof...(args),args...>::doIt (vec,s,e,id);
	  for (auto& s : vec){
	    if (!s)
	      return nullptr;
	  }
	  return std::make_shared<State<sizeof... (args)>> (vec);
	}
      };

	  template<class A>
	  struct TagElement {
		typename A::Storage::StorageTag tag;
	  };
	  
	  template<class ...args>
	  struct StoreTag : public TagElement<args>... {
		template<class A>
		  auto& getSubTag () {return TagElement<A>::tag;}
	  };

	  template<class A>
	  struct StoreElement {
		typename A::Storage store;
	  };
	  
	  template<class ...args>
	  struct Storing : public StoreElement<args>... {
		template<class A>
		  auto& getSubStore () {return StoreElement<A>::store;}
	  };
	  
	  
      template<class... args>
      class Storer : public MiniMC::CPA::Storer {
      public:
	using StorageTag = StoreTag<args...>;
	
	template<size_t statenb,class A, class ... As>
	struct Saver {
	  static void doSave (const State_ptr& state, StorageTag& t, Storing<args...> store) {
	    auto& ss = static_cast<State<sizeof... (args)>& > (*state);
	    store.template getSubStore<A> ().saveState (ss.template get<statenb> (),&t.template getSubTag<A> ());
	    Saver<statenb,As...>::doSave (state,t,store);
	  }
	};
	
	template<size_t statenb,class A>
	struct Saver<statenb,A> {
	  static void doSave (const State_ptr& state, StorageTag& t, Storing<args...> store) {
	    auto& ss = static_cast<State<sizeof... (args)>& > (*state);
	    store.template getSubStore<A> ().saveState (ss.template get<statenb> (),&t.template getSubTag<A> ());
	  }
	};
	
	
	bool saveState (const State_ptr& state,StorageTag* tag = nullptr) {
	  MiniMC::Hash::hash_t hash= state->hash(0);
	  if (tag) {
	    Saver<0,args...>::doSave (state,*tag,store);
	  }
	  if (stored.count(hash))
	    return false;
	  else {
	    stored.insert(hash);
	    return true;
	  }
	}

	template<size_t statenb,class A, class ... As>
	struct Loader {
	  static void doLoad (std::vector<State_ptr>& vec, StorageTag& t, Storing<args...> store) {
	    vec.push_back(store.template getSubStore<A> ().loadState (t.template getSubTag<A> ()));
	    Loader<statenb,As...>::doLoad (vec,t,store);
	  }
	};
	
	template<size_t statenb,class A>
	struct Loader<statenb,A> {
	  static void doLoad (std::vector<State_ptr>& vec, StorageTag& t, Storing<args...> store) {	    
	    vec.push_back(store.template getSubStore<A> ().loadState (t.template getSubTag<A> ()));
	  }
	};
	
	State_ptr loadState (StorageTag tag) {
	  std::vector<State_ptr> vec;
	  vec.reserve (sizeof... (args));
	  Loader<0,args...>::doLoad (vec,tag,store);
	  return std::make_shared<State<sizeof... (args)>> (vec);
	}
	
      private:
	std::set<MiniMC::Hash::hash_t> stored;
	Storing<args...> store;
      };
      
      
      template<size_t ask,class... CPAs>
      struct CPADef {
	using Query = StateQuery<ask,CPAs...>;
	using Transfer = Transferer<CPAs...>;
	using Join = MiniMC::CPA::Joiner;
	using Storage = Storer<CPAs...>;
      };
    }
  }
}

#endif
