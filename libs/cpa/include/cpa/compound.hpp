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
	  for (auto& state : states) 
	    hash = state->hash(hash);
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
	  return std::make_shared<State<sizeof... (args)>> (vec);
	}
      };

      
      
      template<class... args>
      class Storer : public MiniMC::CPA::Storer {
      public:
	using StorageTag = std::tuple<args...>;
	
	bool saveState (const State_ptr& state,StorageTag* tag = nullptr) {
	  MiniMC::Hash::hash_t hash= state->hash(0);
	  if (stored.count(hash))
	    return false;
	  
	  return true;
	}
	State_ptr loadState (StorageTag) {
	  return nullptr;
	}
      private:
	std::set<MiniMC::Hash::hash_t> stored;
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
