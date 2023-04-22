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

    template<class State>
    class TTransfer {
    public:
      virtual ~TTransfer () {}
      virtual State_ptr<State> doTransfer(const State&, const MiniMC::Model::Edge&, proc_id) {return nullptr;}
    };
    
    
    using Transferer = TTransfer<CommonState>;
    using DataTransferer = TTransfer<DataState>;
    using CFATransferer = TTransfer<CFAState>;
    
    
    
    template<class State>
    using TTransferer_ptr = std::shared_ptr<TTransfer<State>>;
    
    using Transferer_ptr = TTransferer_ptr<CommonState>;
    
    
    template<class State>
    class TJoiner {
    public:
      virtual ~TJoiner () {}
      
      virtual State_ptr<State> doJoin(const State&, const State&) {return nullptr;}
      
    };

    
    using Joiner = TJoiner<CommonState>;
    using DataJoiner = TTransfer<DataState>;
    using CFAJoiner = TTransfer<CFAState>;
    
    template<class State>
    using TJoiner_ptr = std::shared_ptr<TJoiner<State>>;
    
    using Joiner_ptr = TJoiner_ptr<CommonState>;
    
    template<class T = CommonState>
    struct ICPA {
      virtual ~ICPA() {}
      virtual State_ptr<T> makeInitialState(const InitialiseDescr&) = 0;
      virtual TTransferer_ptr<T> makeTransfer(const MiniMC::Model::Program& ) const = 0;
      virtual TJoiner_ptr<T> makeJoin() const {return std::make_shared<TJoiner<T>> ();}
      
    };
    
    using CPA_ptr = std::shared_ptr<ICPA<CommonState>>;    
    
    class AnalysisTransfer {
    public:
      AnalysisTransfer (Transferer_ptr&& locTransfer, std::vector<Transferer_ptr>&& dtransfers) : locTransfer(std::move(locTransfer)), dataTransfers(std::move(dtransfers)) {}
      bool Transfer (const AnalysisState&, const MiniMC::Model::Edge&, proc_id,AnalysisState&);
    private:
      Transferer_ptr locTransfer;
      std::vector<Transferer_ptr> dataTransfers;    
    };
    
    
    class AnalysisBuilder {
    public:
      AnalysisBuilder (CPA_ptr&& cpa) : cfa_cpa(std::move(cpa)) {}
      void addDataCPA (CPA_ptr&& cpa) {data_cpa.push_back (std::move(cpa));}
      AnalysisTransfer makeTransfer (const MiniMC::Model::Program& prgm) const  {
	std::vector<Transferer_ptr> datas;
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
      CPA_ptr cfa_cpa;
      std::vector<CPA_ptr> data_cpa;
    };
    
  } // namespace CPA
} // namespace MiniMC

#endif
