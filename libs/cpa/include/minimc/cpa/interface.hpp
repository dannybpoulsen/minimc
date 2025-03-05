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
    
    class Transfer {
    public:
      virtual ~Transfer () {}
      virtual std::generator<State_ptr> doTransfer(const State&, const Transition&) = 0;
    };
    
    
    
    
    
    using Transferer_ptr = std::shared_ptr<Transfer>;
    
    
    
    struct ICPA {
      virtual ~ICPA() {}
      virtual State_ptr makeInitialState(const InitialiseDescr&) = 0;
      virtual Transferer_ptr makeTransfer(const MiniMC::Model::Program& ) const = 0;
    };
    
    using TCPA_ptr = std::shared_ptr<ICPA>;    
    
    class AnalysisTransfer {
    public:
      AnalysisTransfer (std::vector<Transferer_ptr>&& dtransfers) : dataTransfers(std::move(dtransfers)) {}
      bool Transfer (const AnalysisState&, const Transition&, AnalysisState&);
    private:
      std::vector<Transferer_ptr> dataTransfers;    
    };
    
    enum class CPAType {
      Concrete,
      Pathformula
    };

    template<CPAType type,class... Args>
    TCPA_ptr makeCPA (Args... args);
    
    class AnalysisBuilder {
    public:
      template<CPAType t,class... Args>
      auto& add (Args&&... args) {
	data_cpa.push_back (makeCPA<t> (std::forward<Args> (args)...));//std::make_shared<T> (std::forward<Args> (args)...));
	return *this;
      }
      
      AnalysisTransfer makeTransfer (const MiniMC::Model::Program& prgm) const  {
	std::vector<Transferer_ptr> datas;
	for (auto& d : data_cpa)
	  datas.push_back (d->makeTransfer (prgm));
	return AnalysisTransfer (std::move(datas));
      }
      
      AnalysisState makeInitialState (const InitialiseDescr& descr) const  {
	std::vector<State_ptr> datas;
	for (auto& d : data_cpa) 
	  datas.push_back (d->makeInitialState (descr));
	return AnalysisState (std::move(datas));
      }
      
    private:
      std::vector<TCPA_ptr> data_cpa;
    };
    
  } // namespace CPA
} // namespace MiniMC

#endif
