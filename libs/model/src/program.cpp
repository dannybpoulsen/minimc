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
      auto copyVariables ( const MiniMC::Model::RegisterDescr& vars) {
	MiniMC::Model::RegisterDescr_uptr stack= std::make_unique<MiniMC::Model::RegisterDescr> ();
	for (auto& v : vars.getRegisters ())
	  stack->addRegister (v->getSymbol(),v->getType ());
	return stack;
      }
      
      MiniMC::Model::InstructionStream copyInstructionStream (const MiniMC::Model::InstructionStream& instr,
							   const MiniMC::Model::RegisterDescr* vars = nullptr
							   ) {
	auto replacer = [&vars](auto& v) -> MiniMC::Model::Value_ptr {
	  if (v == nullptr)
	    return v;
	  if (v->isConstant ()) {
	    return v;
	  }
	  else if (vars) {
	    return vars->getRegisters ().at (std::static_pointer_cast<MiniMC::Model::Register> (v)->getId ());
	  }
	  else {
	    throw MiniMC::Support::Exception ("Cannot replace variable");
	  }
	};
	std::vector<Instruction> instrs;
	for (auto& t : instr) {
	  std::back_inserter (instrs) = MiniMC::Model::copyInstructionWithReplace (t, replacer);
	}
	return {instrs, instr.isPhi ()};

      }
      
      auto copyCFA (const MiniMC::Model::CFA& cfa,
		    const MiniMC::Model::RegisterDescr& vars
		    ) {
	MiniMC::Model::CFA ncfa;
	std::unordered_map<Location_ptr, Location_ptr> locMap;

	for (auto& loc : cfa.getLocations ()) {
	  locMap.emplace (loc,ncfa.makeLocation (loc->getInfo ()));
	}

	ncfa.setInitial (locMap.at(cfa.getInitialLocation ()));
	
	for (auto& e : cfa.getEdges ()) {
	  auto nedge = ncfa.makeEdge (locMap.at (e->getFrom ()), locMap.at (e->getTo ()));

	  if (e->getInstructions ()) {
	    auto& instrs = e->getInstructions();
	    nedge->getInstructions () = copyInstructionStream (instrs,&vars);
	      
	  }
	  
	}
	
	return ncfa;
	
      }

      auto copyFunction (const Function_ptr& function) {
	auto varstack = copyVariables (function->getRegisterStackDescr ());
	std::vector<Register_ptr> parameters;
	std::for_each (function->getParameters().begin (),
		       function->getParameters ().end(),
		       [&](auto& vv) {parameters.push_back (varstack->getRegisters ().at (vv->getId ()));}
		       );
	auto cfa = copyCFA (function->getCFA (),*varstack);
	auto retType  = function->getReturnType ();
	return program.addFunction (function->getSymbol ().getName(),
				    parameters,
				    retType,
				    std::move(varstack),
				    std::move(cfa));
      }

      
      
      void copyProgram (const Program& p) {
	for (auto f : p.getFunctions ()) {
	  copyFunction (f);
	}
	
	for (auto f: p.getEntryPoints ()) {
	  program.addEntryPoint (f->getSymbol ().getName());
	}

	program.setInitialiser (copyInstructionStream(p.getInitialiser ()));
	
	program.getHeapLayout () = p.getHeapLayout ();
	
      }
      
      Program& program;
    };
    

    Program::Program (const Program& p) : Program(p.tfact,p.cfact) {
      Copier{*this}.copyProgram(p);
    }
  }
}
