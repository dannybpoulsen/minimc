#include <iostream>

#include "minimc/model/instructions.hpp"
#include "minimc/model/modifications/instrument_loads.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/support/workinglist.hpp"
#include "minimc/support/overload.hpp"

namespace MiniMC {
  namespace Model {
    namespace Modifications {
      bool InstrumentLoads::runFunction(const MiniMC::Model::Function_ptr& F) {
	auto& cfg = F->getCFA();
	for (auto& e : cfg.getEdges()) {
	  InstructionStream stream;
	  auto& instr = e->getInstructions();
	  auto it = instr.begin();
	  for (; it != instr.end();++it) {
	    
	    it->visitExpressions ([&stream](auto& expr) {
	      visitSubExpressions (MiniMC::Support::Overload {
		  [&stream](MiniMC::Model::LoadExpr& load) {
		    MiniMC::Model::ExpressionBuilder builder;
		    (builder << load.mem().shared_from_this() << load.addr().shared_from_this()).ValidPointer();
		    stream.add<VMInstructionCode::Assert> (builder.get());
		    (builder << load.mem().shared_from_this() << load.addr().shared_from_this()).I64(load.loadType()->getSize()).PtrAdd().ValidPointer();
		    stream.add<VMInstructionCode::Assert> (builder.get());
		  },
		    [&stream](MiniMC::Model::StoreExpr& store) {
		    MiniMC::Model::ExpressionBuilder builder;
		    (builder << store.storeto().shared_from_this() << store.addr().shared_from_this()).ValidPointer();
		    stream.add<VMInstructionCode::Assert> (builder.get());
		    (builder << store.storeto().shared_from_this() << store.addr().shared_from_this()).I64(store.storee().getType()->getSize()).PtrAdd().ValidPointer();
		    stream.add<VMInstructionCode::Assert> (builder.get());
		    },
		    MiniMC::Support::Ignore {}
		}
		, *expr);
	      
	    }
	      
	      ); 
	    
	    
	  }
	  instr.insert (instr.begin(),stream.begin(),stream.end());
	    
	}
	     
	
	return true;
      }
      
    }
  }
}

