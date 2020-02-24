#include <gsl/pointers>

#include "cpa/concrete_no_mem.hpp"
#include "stack.hpp"
#include "heap.hpp"
#include "state.hpp"
#include "support/types.hpp"
#include "support/exceptions.hpp"
#include "instructionimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      MiniMC::CPA::State_ptr StateQuery::makeInitialState (const MiniMC::Model::Program& prgm)  {
	Heap heap;
	std::vector<pointer_t> stacks;
	auto gsize = prgm.getGlobals()->getTotalSize();
	std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[gsize]);
	auto gstack = createStack (prgm.getGlobals().get(),heap);
	for (auto& entry : prgm.getEntryPoints ()) {
	  auto stackDescr = entry->getVariableStackDescr ();
	  stacks.push_back (createStack(stackDescr,heap));
	}
	return std::make_shared<MiniMC::CPA::ConcreteNoMem::State> (gstack,stacks,heap);
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
	auto cdet = state->getStackDetails (id);
	auto det = nstate->getStackDetails (id);
	if (e->hasAttribute<MiniMC::Model::AttributeType::Guard> ()) {
	  auto& guard = e->getAttribute<MiniMC::Model::AttributeType::Guard> ();
	  RegisterLoader loader (cdet, guard.guard);
	  bool val = loader.getRegister().template get<MiniMC::uint8_t> ();
	  if (guard.negate)
	    val = !val;
	  if (!val)
	    return nullptr;
	  
	}
	try {
	  if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
	    auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
	    gsl::not_null<const MiniMC::CPA::ConcreteNoMem::State::StackDetails*> datFrom = instr.isPhi ? &cdet : &det;
	    for (auto& inst : instr) {
	      switch (inst.getOpcode ()) {
#define X(OP)								\
		case MiniMC::Model::InstructionCode::OP:		\
		  ExecuteInstruction<MiniMC::Model::InstructionCode::OP>::execute (*datFrom,det,inst); \
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
	  }
	  det.commit ();
	  return nstate;
	}
	catch  (MiniMC::Support::AssumeViolated) {
	  return nullptr;
	}
      }
      
    }
  }
}
