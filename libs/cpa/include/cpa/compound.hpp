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

		virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const override  {
		  MiniMC::Hash::hash_t hash = seed;
		  
		  for (auto& state : states) {
			MiniMC::Hash::hash_combine (hash,*state);
		  }
		  return hash;
		}
		
		bool need2Store () const {
		  for (auto& state : states) {
			if (state->need2Store ()) {
			  return true;
			}
		  }
		  return false;
			
		}

		bool ready2explore () const override {
		  for (auto& state : states) {
			if (!state->ready2explore ()) {
			  return false;
			}
		  }
		  return true;
		  
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

		virtual const Concretizer_ptr getConcretizer () {
		  
		  return this->template get<1> ()->getConcretizer ();
		  //return Concretizer_ptr (this);
		}
		
		
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
		template<size_t i, class Inserter>
		static bool buildVector (Inserter& ins, const State<sizeof...(args)>& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
		  static_assert(i <= sizeof... (args));
		  if constexpr (i == sizeof... (args)) {
			return true; //We are done
		  }
		  else {
			using A = typename GetNthTemplateArgument<i,args...>::Temp;
			auto res = A::Transfer::doTransfer (s.template get<i> (),e,id);
			if (res) {
			  ins = res;
			  return buildVector<i+1,Inserter> (ins,s,e,id);
			}
			else
			  return false;
		  }
		
		}
		
		static State_ptr doTransfer (const State_ptr& a, const MiniMC::Model::Edge_ptr& e,proc_id id) {
		  auto s = static_cast<State<sizeof... (args)>&> (*a);
		  std::vector<MiniMC::CPA::State_ptr> vec;
		  auto ins = std::back_inserter(vec);
		  //BuildVector<0,sizeof...(args),args...>::doIt (vec,s,e,id);
		  if (buildVector<0,decltype(ins)> (ins,s,e,id)) {
			return std::make_shared<State<sizeof... (args)>> (vec);	
		  }
		  return nullptr;
		  
		}
      };

	  
	  
      template<class... Args>
      struct Joiner {
		template<size_t i, class Inserter>
		static bool buildVector (Inserter& ins, const State<sizeof...(Args)>& l, const State<sizeof...(Args)>& r) {
		  static_assert(i <= sizeof... (Args));
		  if constexpr (i == sizeof... (Args)) {
			return true; //We are done
		  }
		  else {
			using A = typename GetNthTemplateArgument<i,Args...>::Temp;
			auto left = l.template get<i> ();
			auto right = r.template get<i> ();
			auto res = A::Join::doJoin (left,right);
		    
			if (res) {
			  ins = res;
			  return buildVector<i+1,Inserter> (ins,l,r);
			}
			else
			  return false;
		  }
		  
		}
	    
		static State_ptr doJoin (const State_ptr& l, const State_ptr& r) {
		  
		  auto& left = static_cast<State<sizeof... (Args)>&> (*l);
		  auto& right = static_cast<State<sizeof... (Args)>&> (*r);

	  
		  std::vector<MiniMC::CPA::State_ptr> vec;
		  auto ins = std::back_inserter(vec);
		  if (buildVector<0,decltype(ins)> (ins,left,right)) {
			return std::make_shared<State<sizeof... (Args)>> (vec);	
		  }
		  return nullptr;
	      
		}
	    
		template<size_t i>
		static bool checkCovers (const State<sizeof...(Args)>& l, const State<sizeof...(Args)>& r) {
		  static_assert(i <= sizeof... (Args));
		  if constexpr (i == sizeof... (Args)) {
			return true; //We are done
		  }
		  else {
			using A = typename GetNthTemplateArgument<i,Args...>::Temp;
			auto left = l.template get<i> ();
			auto right = r.template get<i> ();
			
			
			if (A::Join::covers (left,right)) {
			  return checkCovers<i+1> (l,r);
			}
			else {
			  return false;
			}
		  }
		}
	    
		static bool covers (const State_ptr& l, const State_ptr& r) {
		  auto left = static_cast<State<sizeof... (Args)>&> (*l);
		  auto right = static_cast<State<sizeof... (Args)>&> (*r);
		  return checkCovers<0> (left,right);
		}
		
      };
	  
      template<class A, class... CPAs>
      struct PreValidateSetup {
		
		static bool validate (const MiniMC::Model::Program& prgm, MiniMC::Support::Messager& mess) {
		  A::PreValidate::validate (prgm,mess);
		  return PreValidateSetup<CPAs...>::validate (prgm,mess);
		}
      };

      template<class A>
      struct PreValidateSetup<A> {
		static bool validate (const MiniMC::Model::Program& seq, MiniMC::Support::Messager& mess) {
		  return A::PreValidate::validate (seq,mess);
		}
      };
	  
      template<size_t ask,class... CPAs>
      struct CPADef {
		using Query = StateQuery<ask,CPAs...>;
		using Transfer = Transferer<CPAs...>;
		using Join = Joiner<CPAs...>;
		using Storage = Storer<Join>;
		using PreValidate = PreValidateSetup<CPAs...>;
      };
    }
  }
}

#endif
