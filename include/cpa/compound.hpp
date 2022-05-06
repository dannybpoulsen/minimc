#ifndef _COMPOUND__
#define _COMPOUND__

#include "cpa/interface.hpp"
#include "hash/hashing.hpp"
#include <initializer_list>
#include <set>
#include <vector>

namespace MiniMC {
  namespace CPA {
    namespace Compounds {
      class State : public MiniMC::CPA::State {
      public:
        State(std::initializer_list<MiniMC::CPA::State_ptr> l) {
          std::copy(l.begin(), l.end(), std::back_inserter(states));
        }

        State(std::vector<MiniMC::CPA::State_ptr>&& l) : states(std::move(l)) {
        }

	MiniMC::Hash::hash_t hash() const override {
          MiniMC::Hash::hash_t hash{0};
	  
          for (auto& state : states) {
            MiniMC::Hash::hash_combine(hash, *state);
	  }
	  return hash;
        }
	
	const MiniMC::CPA::LocationInfo& getLocationState () const override {
	  return states[0]->getLocationState ();
	}
        
        virtual std::ostream& output(std::ostream& os) const override {
          for (auto& state : states) {
            state->output(os);
            os << "\n________________\n";
          }
          return os;
        }

        const State_ptr& get(size_t i) const { return states.at(i); }

	auto begin () const {
	  return states.begin ();
	}

	auto end () const {
	  return states.end ();
	}
	
        virtual const Solver_ptr getConcretizer() const override {

          return this->get(1)->getConcretizer();
        }

        virtual std::shared_ptr<MiniMC::CPA::State> copy() const override {
          std::vector<MiniMC::CPA::State_ptr> copies;
          std::for_each(states.begin(), states.end(), [&](auto& s) { copies.push_back(s->copy()); });
          return std::make_shared<State>(std::move(copies));
        }

	ByteVectorExpr_ptr symbEvaluate (proc_id id, const MiniMC::Model::Register_ptr& v) const override  {
	  return this->get(1)->symbEvaluate (id,v);  
	}
	
      private:
        std::vector<MiniMC::CPA::State_ptr> states;
      };

      
      struct StateQuery : public MiniMC::CPA::StateQuery {
        StateQuery(std::vector<MiniMC::CPA::StateQuery_ptr>&& pts) : sub_queries(std::move(pts)) {}
        State_ptr makeInitialState(const InitialiseDescr& descr) {
          //std::initializer_list<MiniMC::CPA::State_ptr> init ( {(args::Query::makeInitialState (prgm)) ...});
          //return std::make_shared<State<sizeof... (args)>> (init);
          std::vector<MiniMC::CPA::State_ptr> statees;
          auto inserter = std::back_inserter(statees);
          std::for_each(sub_queries.begin(), sub_queries.end(), [&inserter, &descr](auto& it) { inserter = (it->makeInitialState(descr)); });
          return std::make_shared<State>(std::move(statees));
        }
	
      private:
        std::vector<MiniMC::CPA::StateQuery_ptr> sub_queries;
      };

      
      struct Transferer : public MiniMC::CPA::Transferer {
        Transferer(std::vector<MiniMC::CPA::Transferer_ptr>&& pts) : transfers(std::move(pts)) {}
        State_ptr doTransfer(const State_ptr& a, const MiniMC::Model::Edge_ptr& e, proc_id id) {
          auto s = static_cast<State&>(*a);
          
	  auto tit = transfers.begin ();
	  auto tend = transfers.end ();
	  auto sit = s.begin ();
	  
	  
	  std::vector<MiniMC::CPA::State_ptr> vec;
          for (; tit != tend; ++tit,++sit) {
            auto res = (*tit)->doTransfer(*sit, e, id);
            if (!res) {
              return nullptr;
            }
            vec.push_back(res);
          }
          return std::make_shared<State>(std::move(vec));
        }

        std::vector<MiniMC::CPA::Transferer_ptr> transfers;
      };

      struct Joiner : public MiniMC::CPA::Joiner {
	Joiner(std::vector<MiniMC::CPA::Joiner_ptr>&& pts) : joiners(std::move(pts)) {}
								     
        State_ptr doJoin(const State_ptr& l, const State_ptr& r) {
          auto left = static_cast<State&>(*l);
          auto right = static_cast<State&>(*r);
          auto n = joiners.size();
          std::vector<MiniMC::CPA::State_ptr> vec;
          for (size_t i = 0; i < n; i++) {
            auto res = joiners[i]->doJoin(left.get(i), right.get(i));
            if (!res) {
              return nullptr;
            }
            vec.push_back(res);
          }
          return std::make_shared<State>(std::move(vec));
        }

        
        bool covers(const State_ptr& l, const State_ptr& r) {
          auto left = static_cast<State&>(*l);
          auto right = static_cast<State&>(*r);
          auto n = joiners.size();
          for (size_t i = 0; i < n; i++) {
            auto res = joiners[i]->covers(left.get(i), right.get(i));
            if (!res) {
              return false;
            }
          }
          return true;
        }

      private:
        std::vector<MiniMC::CPA::Joiner_ptr> joiners;
      };

     

      
      struct CPA : public MiniMC::CPA::ICPA {
        CPA(std::vector<MiniMC::CPA::CPA_ptr>&& p) : cpas(std::move(p)) {
        }

        CPA(std::initializer_list<MiniMC::CPA::CPA_ptr> p) {
          std::copy(p.begin(), p.end(), std::back_inserter(cpas));
        }
	MiniMC::CPA::StateQuery_ptr makeQuery() const override {
          std::vector<StateQuery_ptr> queries;
          std::for_each(cpas.begin(), cpas.end(), [&queries](auto& it) { queries.push_back(it->makeQuery()); });
          return std::make_shared<StateQuery>(std::move(queries));
        }
	MiniMC::CPA::Transferer_ptr makeTransfer() const override {
          std::vector<Transferer_ptr> transfers;
          std::for_each(cpas.begin(), cpas.end(), [&transfers](auto& it) { transfers.push_back(it->makeTransfer()); });
          return std::make_shared<Transferer>(std::move(transfers));
        }
	MiniMC::CPA::Joiner_ptr makeJoin() const override {
          std::vector<Joiner_ptr> joiners;
          std::for_each(cpas.begin(), cpas.end(), [&joiners](auto& it) { joiners.push_back(it->makeJoin()); });
          return std::make_shared<Joiner>(std::move(joiners));
        }
        
      private:
        std::vector<MiniMC::CPA::CPA_ptr> cpas;
      };

    } // namespace Compounds
  }   // namespace CPA
} // namespace MiniMC

#endif
