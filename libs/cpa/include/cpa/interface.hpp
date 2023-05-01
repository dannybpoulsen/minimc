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

#include "cpa/state.hpp"
#include "hash/hashing.hpp"
#include "model/cfg.hpp"
#include "support/feedback.hpp"
#include "support/sequencer.hpp"
#include <memory>
#include <ostream>
#include <unordered_map>

namespace MiniMC {
  namespace CPA {

    struct InitialiseDescr {
    public:
      InitialiseDescr(std::vector<MiniMC::Model::Function_ptr> entries,
                      MiniMC::Model::HeapLayout heap,
                      MiniMC::Model::InstructionStream init,
                      const MiniMC::Model::Program& program) : entries(std::move(entries)),
                                                               heap(std::move(heap)),
                                                               init(std::move(init)),
                                                               prgm(program) {}

      auto& getEntries() const { return entries; }
      auto& getHeap() const { return heap; }
      auto& getInit() const { return init; }
      auto& getProgram() const { return prgm; }
      
    private:
      std::vector<MiniMC::Model::Function_ptr> entries;
      MiniMC::Model::HeapLayout heap;
      MiniMC::Model::InstructionStream init;
      const MiniMC::Model::Program& prgm;
    };

    struct Transition {
      Transition (MiniMC::Model::Edge* e, MiniMC::proc_t p) : edge(e),proc(p) {}
      Transition ( const Transition&) = default;
      MiniMC::Model::Edge* edge;
      MiniMC::proc_t proc;
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
      virtual TJoiner_ptr<T> makeJoin() const {return std::make_shared<TJoiner<T>> ();}
      
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
