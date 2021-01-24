#include <gsl/pointers>

#include "util/vm.hpp"
#include "cpa/concrete.hpp"
#include "instructionimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {

	  class State : public MiniMC::CPA::State {
	  public:
		State (const VariableLookup& g, const std::vector<VariableLookup>& var) : globals(g),proc_vars(var) {
		  
		}
		virtual std::ostream& output (std::ostream& os) const {
		  os << "Globals\n";
		  os << globals << "\n";
		  for (auto& vl : proc_vars) {
			os << "===\n";
			os << vl << "\n";
		  }
		  return os << "==\n";
		}
		
		virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const {
		  return reinterpret_cast<MiniMC::Hash::hash_t> (this);
		}
		virtual std::shared_ptr<MiniMC::CPA::State> copy () const {
		  return std::make_shared<State> (*this);
		}

		auto& getGlobals () {return globals;}
		auto& getProc (std::size_t i) {return proc_vars[i];}

		auto& getGlobals () const {return globals;}
		auto& getProc (std::size_t i) const  {return proc_vars[i];}
		
		
		virtual bool need2Store () const {return false;}
		virtual bool ready2explore () const {return true;}
	  private:
		VariableLookup globals;
		std::vector<VariableLookup> proc_vars;
	  };
	  
	  MiniMC::CPA::State_ptr  StateQuery::makeInitialState (const MiniMC::Model::Program& p) {
		VariableLookup globals (p.getGlobals()->getTotalVariables ());
		for (auto& v : p.getGlobals ()->getVariables ()) {
		  globals[v] = MiniMC::Util::Array (v->getType ()->getSize ());
		}

		std::vector<VariableLookup> stack;
		for (auto& f : p.getEntryPoints ()) {
		  auto& vstack = f->getVariableStackDescr ();
		  stack.emplace_back (vstack->getTotalVariables ());
		  for (auto& v : vstack->getVariables ()) {
			MiniMC::Util::Array arr (v->getType ()->getSize ());
			stack.back()[v] = arr;
			assert(stack.back()[v].getSize () == v->getType()->getSize ());
		  }
		}

		auto state =  std::make_shared<State> (globals,stack);
		
		
		VMData data {
		  .readFrom = {
			.global = const_cast<VariableLookup*> (&state->getGlobals ()),
			.local = nullptr
		  },
		  .writeTo = {
			.global = &state->getGlobals (),
			.local = nullptr
		  }
		};
		
		auto it = p.getInitialisation ().begin();
		auto end = p.getInitialisation ().end();
		MiniMC::Util::runVM<decltype(it),VMData,ExecuteInstruction> (it,end,data);
		
		return state;
		
	  }
	  

	  MiniMC::CPA::State_ptr Transferer::doTransfer (const MiniMC::CPA::State_ptr& s, const MiniMC::Model::Edge_ptr& e,proc_id id) {
		auto resstate = s->copy ();
		auto& ostate = static_cast<const MiniMC::CPA::Concrete::State&> (*s);
		auto& nstate = static_cast<MiniMC::CPA::Concrete::State&> (*resstate);

		VMData data {
		  .readFrom = {
			.global = const_cast<VariableLookup*> (&nstate.getGlobals ()),
			.local = const_cast<VariableLookup*> (&nstate.getProc (id))
		  },
		  .writeTo = {
			.global = &nstate.getGlobals (),
			.local = &nstate.getProc (id)
		  }
		};
		
		if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  
		  auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
		  try {
			
			if (instr.isPhi) {
			  data.readFrom.global = const_cast<VariableLookup*> (&ostate.getGlobals ());
			  data.readFrom.local = const_cast<VariableLookup*> (&ostate.getProc(id));
			  
			}
			auto it = instr.begin();
			auto end = instr.end ();
									
			MiniMC::Util::runVM<decltype(it),VMData,ExecuteInstruction> (it,end,data);
			
			
		  }
		  catch  (MiniMC::Support::AssumeViolated) {
			return nullptr;
		  }
		  
		}
		return resstate;
	  }

	}
  }
}
