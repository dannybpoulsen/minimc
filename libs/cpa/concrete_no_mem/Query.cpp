#include "cpa/concrete_no_mem.hpp"
#include "stack.hpp"
#include "state.hpp"
#include "support/types.hpp"
#include "support/exceptions.hpp"
#include "instructionimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      MiniMC::CPA::State_ptr StateQuery::makeInitialState (const MiniMC::Model::Program& prgm)  {
		std::vector<Stack> stacks;
		assert(prgm.getGlobals()->getTotalSize() == 0);
		for (auto& entry : prgm.getEntryPoints ()) {
		  auto stackDescr = entry->getVariableStackDescr ();
		  auto size = stackDescr->getTotalSize();
		  std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[size]);
		  stacks.emplace_back (buffer,size);
		}
		return std::make_shared<MiniMC::CPA::ConcreteNoMem::State> (stacks);
      }
      
      size_t nbOfProcesses (const State_ptr& s) {
		auto& state = static_cast<MiniMC::CPA::ConcreteNoMem::State&> (*s);
		return  state.nbProcs  ();
      }
      MiniMC::Model::Location_ptr getLocation (const State_ptr&, proc_id){
	throw MiniMC::Support::Exception ("SHould not be called");
      }

      
      
      MiniMC::CPA::State_ptr Transferer::doTransfer (const State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
	auto state = static_cast<const State*> (s.get ());
	std::cerr << "ID " << id << " " << state->nbProcs ()<< std::endl; 
	assert (id < state->nbProcs ());
	auto nstate = state->lcopy ();
	auto& stack = nstate->getStack (id);
	for (auto& inst : e->getInstructions ()) {
	  switch (inst.getOpcode ()) {
#define X(OP)								\
	    case MiniMC::Model::InstructionCode::OP:			\
	      ExecuteInstruction<MiniMC::Model::InstructionCode::OP>::execute (stack,inst); \
	      break;									
	    TACOPS
	      CASTOPS
	      MEMORY
	      INTERNAL
	      POINTEROPS
	      AGGREGATEOPS
	      }
	}
	return nstate;
      }
      
      
    }
  }
}
