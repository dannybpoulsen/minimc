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
	std::vector<AllocState> allocs;
	auto gsize = prgm.getGlobals()->getTotalSize();
	std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[gsize]);
	Stack globals (buffer,gsize);
	AllocState galloc;
	galloc.push_frame(prgm.getGlobals()->getTotalSize(),globals);
	for (auto& entry : prgm.getEntryPoints ()) {
	  auto stackDescr = entry->getVariableStackDescr ();
	  auto size = stackDescr->getTotalSize();
	  std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[size]);
	  stacks.emplace_back (buffer,size);
	  allocs.emplace_back();
	  bool res = allocs.back ().push_frame(stackDescr->getTotalSize(),stacks.back());
	  assert(res);
	}
	return std::make_shared<MiniMC::CPA::ConcreteNoMem::State> (globals,galloc,stacks,allocs);
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
	assert (id < state->nbProcs ());
	auto nstate = state->lcopy ();
	auto det = nstate->getStackDetails (id);
	if (e->getGuard ()) {
	  RegisterLoader loader (det, e->getGuard ());
	  bool val = loader.getRegister().template get<MiniMC::uint8_t> ();
	  if (e->negatedGuard ())
	    val = !val;
	  if (!val)
	    return nullptr;
	  
	}
	for (auto& inst : e->getInstructions ()) {
	  switch (inst.getOpcode ()) {
#define X(OP)								\
	    case MiniMC::Model::InstructionCode::OP:			\
	      ExecuteInstruction<MiniMC::Model::InstructionCode::OP>::execute (det,inst); \
	      break;									
	    TACOPS
	      COMPARISONS
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
