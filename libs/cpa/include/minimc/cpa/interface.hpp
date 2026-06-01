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
      InitialiseDescr(std::vector<MiniMC::Model::Symbol> entri_func,
                      MiniMC::Model::HeapLayout heap,
                      const MiniMC::Model::Program& program) : heap(std::move(heap)),
                                                               prgm(program) {
	
	for (auto& F : entri_func) {
	  entries.push_back (std::get<MiniMC::Model::Function_wptr>(F.getUserData()).lock());
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

    enum class CPAType {
      Concrete,
      Symbolic
    };

    template<CPAType type,class... Args>
    TCPA_ptr makeCPA (Args... args);
    
    
  } // namespace CPA
} // namespace MiniMC

#endif
