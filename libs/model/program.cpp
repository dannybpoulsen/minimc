#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"

#include <memory>
#include <unordered_map>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    struct Copier {
      Copier (Program& program) : program(program) { }
      MiniMC::Model::VariableStackDescr_ptr copyVariables ( const MiniMC::Model::VariableStackDescr& vars) {
	auto stack = program.makeVariableStack (vars.getPref ());
	for (auto& v : vars.getVariables ())
	  stack->addVariable (v->getName(),v->getType ());
	return stack;
      }

      MiniMC::Model::InstructionStream copyInstructionStream (const MiniMC::Model::InstructionStream& instr,
							   const MiniMC::Model::VariableStackDescr& vars
							   ) {
	auto replacer = [&vars](auto& v) -> MiniMC::Model::Value_ptr {
	  if (v == nullptr)
	    return v;
	  if (v->isConstant ()) {
	    return v;
	  }
	  else
	    return vars.getVariables ().at (std::static_pointer_cast<MiniMC::Model::Register> (v)->getId ());
	};
	std::vector<Instruction> instrs;
	std::cerr << "===" << std::endl;
	for (auto& t : instr) {
	  std::cerr << t << std::endl;
	  std::back_inserter (instrs) = MiniMC::Model::copyInstructionWithReplace (t, replacer);
	}
	return {instrs, instr.isPhi ()};

      }
      
      auto copyCFA (const MiniMC::Model::CFA_ptr& cfa,
		    const MiniMC::Model::VariableStackDescr& vars
		    ) {
	auto replacer = [&vars](auto& v) -> MiniMC::Model::Value_ptr {
	  if (v->isConstant ()) {
	    return v;
	  }
	  else
	    return vars.getVariables ().at (std::static_pointer_cast<MiniMC::Model::Register> (v)->getId ());
	};

	auto ncfa = program.makeCFG ();
	std::unordered_map<Location_ptr, Location_ptr> locMap;

	for (auto& loc : cfa->getLocations ()) {
	  locMap.emplace (loc,ncfa->makeLocation (loc->getInfo ()));
	}

	ncfa->setInitial (locMap.at(cfa->getInitialLocation ()));

	for (auto& e : cfa->getEdges ()) {
	  auto nedge = ncfa->makeEdge (locMap.at (e->getFrom ()), locMap.at (e->getTo ()));

	  if (e->hasAttribute<AttributeType::Guard> ()) {
	    auto& guard = e->getAttribute<AttributeType::Guard>  ();
	    nedge->setAttribute<AttributeType::Guard> ({replacer(guard.guard),guard.negate});
	  }

	  else if (e->hasAttribute<AttributeType::Instructions> ()) {
	    auto& instrs = e->getAttribute<AttributeType::Instructions>  ();
	    nedge->setAttribute<AttributeType::Instructions> (copyInstructionStream (instrs,vars));
	      
	  }
	  
	}
	
	return ncfa;
	
      }

      auto copyFunction (const Function_ptr& function) {
	auto varstack = copyVariables (*function->getVariableStackDescr ());
	std::vector<Register_ptr> parameters;
	std::for_each (function->getParameters().begin (),
		       function->getParameters ().end(),
		       [&](auto& vv) {parameters.push_back (varstack->getVariables ().at (vv->getId ()));}
		       );
	auto cfa = copyCFA (function->getCFG (),*varstack);
	auto retType  = function->getReturnType ();
	return program.addFunction (function->getName (),
				    parameters,
				    retType,
				    varstack,
				    cfa);
      }

      void copyProgram (const Program& p) {
	auto vstack = program.makeVariableStack ("gg");
	for (auto f : p.getFunctions ()) {
	  copyFunction (f);
	}
	program.getHeapLayout () = p.getHeapLayout ();
	program.setInitialiser (copyInstructionStream (p.getInitialisation (),*vstack));
	
      }
      
      Program& program;
    };
    

    Program::Program (const Program& p) : Program(p.tfact,p.cfact) {
      Copier{*this}.copyProgram(p);
    }
  }
}
