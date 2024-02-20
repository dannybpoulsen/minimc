/**
 * @file   interface.hpp
 * @date   Mon Apr 20 17:18:43 2020
 *
 * MiniMC uses something called CPAs to encapsulate different
 * "verification views" on  a program. For instance one CPA may
 * "track" the program locations of all processes/threads while
 * another tracks their variable values and memory. Combining these
 * two together provides an explicit verification context
 *
 *
 */
#ifndef _CPA_INTERFACE__
#define _CPA_INTERFACE__

#include "minimc/cpa/state.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/cfg.hpp"
#include <memory>
#include <ostream>
#include <unordered_map>

namespace MiniMC {
  namespace CPA {

    struct FunctionInit {
      FunctionInit (MiniMC::Model::Function_ptr function,
		    std::vector<MiniMC::Model::Value_ptr> params = {}) : function(function),
								    params(params) {}
      auto& getFunction () const {return function;}
      auto& getParams () const {return params;}
      MiniMC::Model::Function_ptr function;
      std::vector<MiniMC::Model::Value_ptr> params;
    };
    
    struct InitialiseDescr {
    public:
      InitialiseDescr(std::vector<MiniMC::Model::Function_ptr> entri_func,
                      MiniMC::Model::HeapLayout heap,
                      const MiniMC::Model::Program& program) : heap(std::move(heap)),
                                                               prgm(program) {
	
	for (auto& F : entri_func) {
	  entries.push_back (F);
	}
      }

      auto& getEntries() const { return entries; }
      auto& getHeap() const { return heap; }
    
      auto& getProgram() const { return prgm; }
      
      
    private:
      std::vector<FunctionInit> entries;
      MiniMC::Model::HeapLayout heap;
   
      const MiniMC::Model::Program& prgm;
    };

    struct Transition {
      Transition (MiniMC::Model::Edge* e, MiniMC::Model::proc_t p) : edge(e),proc(p) {}
      Transition ( const Transition&) = default;
      MiniMC::Model::Edge* edge;
      MiniMC::Model::proc_t proc;
    };
    
    template<class State>
    class TTransfer {
    public:
      virtual ~TTransfer () {}
      virtual State_ptr<State> doTransfer(const State&, const Transition&) {return nullptr;}
    };
    
    
    
    
    
    template<class State>
    using TTransferer_ptr = std::shared_ptr<TTransfer<State>>;
    
    
    template<class State>
    class TJoiner {
    public:
      virtual ~TJoiner () {} 
      virtual State_ptr<State> doJoin(const State&, const State&) {return nullptr;}
    };
    
    
    template<class State>
    using TJoiner_ptr = std::shared_ptr<TJoiner<State>>;
    
    template<class T>
    struct ICPA {
      virtual ~ICPA() {}
      virtual State_ptr<T> makeInitialState(const InitialiseDescr&) = 0;
      virtual TTransferer_ptr<T> makeTransfer(const MiniMC::Model::Program& ) const = 0;
      
    };

    template<class State>
    using TCPA_ptr = std::shared_ptr<ICPA<State>>;    

    class AnalysisTransfer {
    public:
      AnalysisTransfer (TTransferer_ptr<CFAState>&& locTransfer, std::vector<TTransferer_ptr<DataState>>&& dtransfers) : locTransfer(std::move(locTransfer)), dataTransfers(std::move(dtransfers)) {}
      bool Transfer (const AnalysisState&, const Transition&, AnalysisState&);
    private:
      TTransferer_ptr<CFAState> locTransfer;
      std::vector<TTransferer_ptr<DataState>> dataTransfers;    
    };
    
    
    class AnalysisBuilder {
    public:
      AnalysisBuilder (TCPA_ptr<CFAState>&& cpa) : cfa_cpa(std::move(cpa)) {}
      void addDataCPA (TCPA_ptr<DataState>&& cpa) {data_cpa.push_back (std::move(cpa));}
      AnalysisTransfer makeTransfer (const MiniMC::Model::Program& prgm) const  {
	std::vector<TTransferer_ptr<DataState>> datas;
	for (auto& d : data_cpa)
	  datas.push_back (d->makeTransfer (prgm));
	return AnalysisTransfer (cfa_cpa->makeTransfer (prgm),std::move(datas));
      }

      
      
      AnalysisState makeInitialState (const InitialiseDescr& descr) const  {
	std::vector<DataState_ptr> datas;
	for (auto& d : data_cpa) 
	  datas.push_back (std::static_pointer_cast<const DataState> (d->makeInitialState (descr)));
	return AnalysisState (std::static_pointer_cast<const CFAState> (cfa_cpa->makeInitialState(descr)),std::move(datas));
      }
      
    private:
      TCPA_ptr<CFAState> cfa_cpa;
      std::vector<TCPA_ptr<DataState>> data_cpa;
    };
    
  } // namespace CPA
} // namespace MiniMC

#endif
