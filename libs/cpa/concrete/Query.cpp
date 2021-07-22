#include <gsl/pointers>
#include <memory>

#include "hash/hashing.hpp"
#include "util/vm.hpp"
#include "cpa/concrete.hpp"
#include "instructionimpl.hpp"
#include "heap.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {      
      class MConcretizer : public MiniMC::CPA::Concretizer {
      public:
	MConcretizer (const VariableLookup& globals,const std::vector<VariableLookup>& v) :  globals(globals),vars(v) {}
	virtual MiniMC::CPA::Concretizer::Feasibility isFeasible () const override { return Feasibility::Feasible;}

	virtual std::ostream&  evaluate_str (proc_id id, const MiniMC::Model::Variable_ptr& var,std::ostream& os) {
	  if (var->isGlobal ()) {
	    return os << globals.at(var);
	  }
	  else {
	    return os << vars.at(id).at(var);
	  }
	}

	virtual MiniMC::Util::Array  evaluate (proc_id id, const MiniMC::Model::Variable_ptr& var) override {
	  if (var->isGlobal ()) {
	    return globals.at(var);
	  }
	  else {
	    return vars.at(id).at(var);
	  }
	}
	
	
      private:
	const VariableLookup& globals;
	const std::vector<VariableLookup>& vars;
		
      };
	  
      class State : public MiniMC::CPA::State, public MiniMC::CPA::Concretizer {
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
		
	virtual MiniMC::Hash::hash_t hash (MiniMC::Hash::seed_t seed = 0) const override {
	  if (!hash_val) {
	    MiniMC::Hash::hash_combine (seed,globals);
	    for (auto& vl : proc_vars) {
	      MiniMC::Hash::hash_combine (seed,vl);
			  
	    }
	    MiniMC::Hash::hash_combine(seed,heap);
	    //uncommnented the update of this buffered hash value. It
	    //disables the buffering as it might be incorrect
	    //The State is really just a container and the parts
	    //making up its hash-values can actually change outside
	    //its knowledge....making it impossible to keep
	    //precomputed hash_value up to date
	    //hash_val = seed;
	    return seed;
	  }
	  return hash_val;
	}
		
	virtual std::shared_ptr<MiniMC::CPA::State> copy () const {
	  return std::make_shared<State> (*this);
	}

	auto& getGlobals () {return globals;}
	auto& getProc (std::size_t i) {return proc_vars[i];}
	auto& getHeap ()   {return heap;}
		
		
	auto& getGlobals () const {return globals;}
	auto& getProc (std::size_t i) const  {return proc_vars[i];}
	auto& getHeap () const  {return heap;}
		
	virtual bool need2Store () const {return false;}
	virtual bool ready2explore () const {return true;}
	virtual bool assertViolated () const { return false;}
		
		
	virtual const Concretizer_ptr getConcretizer () const override {return std::make_shared<MConcretizer> (globals,proc_vars);}
		
		
      private:
	VariableLookup globals;
	std::vector<VariableLookup> proc_vars;
	Heap heap;
	mutable MiniMC::Hash::hash_t hash_val = 0;
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
	    .local = nullptr,
	    .heap = &state->getHeap ()
	  },
	  .writeTo = {
	    .global = &state->getGlobals (),
	    .local = nullptr,
	    .heap = &state->getHeap ()
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
	    .local = const_cast<VariableLookup*> (&nstate.getProc (id)),
	    .heap = &nstate.getHeap ()
	  },
	  .writeTo = {
	    .global = &nstate.getGlobals (),
	    .local = &nstate.getProc (id),
	    .heap = &nstate.getHeap ()
	  }
	};
		
	if (e->hasAttribute<MiniMC::Model::AttributeType::Instructions> ()) {
		  
	  auto& instr = e->getAttribute<MiniMC::Model::AttributeType::Instructions> ();
	  try {
			
	    if (instr.isPhi) {
	      data.readFrom.global = const_cast<VariableLookup*> (&ostate.getGlobals ());
	      data.readFrom.local = const_cast<VariableLookup*> (&ostate.getProc(id));
	      data.readFrom.heap = const_cast<Heap*> (&ostate.getHeap ());
				
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
