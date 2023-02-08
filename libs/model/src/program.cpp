#include "model/cfg.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"

#include <memory>
#include <unordered_map>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    struct Copier {
      using RegReplaceMap = std::unordered_map<Register_ptr,Register_ptr>;
      
      Copier (Program& program) : program(program) { }
      auto copyVariables ( const MiniMC::Model::RegisterDescr& vars, RegReplaceMap& map) {
	MiniMC::Model::RegisterDescr stack;
	for (auto& v : vars.getRegisters ()) {
	  map.emplace (v,stack.addRegister (v->getSymbol(),v->getType ()));
	}
	return stack;
      }
      
      MiniMC::Model::InstructionStream copyInstructionStream (const MiniMC::Model::InstructionStream& instr,
							      const RegReplaceMap& repl
							   ) {
	auto replacer = [&repl](auto& v) -> MiniMC::Model::Value_ptr {
	  if (v == nullptr)
	    return v;
	  if (v->isConstant ()) {
	    return v;
	  }
	  else  {
	    return repl.at (std::static_pointer_cast<MiniMC::Model::Register> (v));
	  }
	  
	};
	std::vector<Instruction> instrs;
	for (auto& t : instr) {
	  std::back_inserter (instrs) = MiniMC::Model::Instruction (t, replacer);
	}
	return {instrs};

      }
      
      auto copyCFA (const MiniMC::Model::CFA& cfa,
		    const RegReplaceMap& vars
		    ) {
	MiniMC::Model::CFA ncfa;
	std::unordered_map<Location_ptr, Location_ptr> locMap;

	for (auto& loc : cfa.getLocations ()) {
	  locMap.emplace (loc,ncfa.makeLocation (loc->getInfo ()));
	}

	ncfa.setInitial (locMap.at(cfa.getInitialLocation ()));
	
	for (auto& e : cfa.getEdges ()) {
	  auto& instrs = e->getInstructions();
	  
	  auto nedge = ncfa.makeEdge (locMap.at (e->getFrom ()), locMap.at (e->getTo ()),copyInstructionStream (instrs,vars));

	  
	}
	
	return ncfa;
	
      }

      auto copyFunction (const Function_ptr& function,RegReplaceMap map) {
	auto varstack = copyVariables (function->getRegisterDescr (),map);
	std::vector<Register_ptr> parameters;
	std::for_each (function->getParameters().begin (),
		       function->getParameters ().end(),
		       [&map,&parameters](auto& vv) {parameters.push_back (map.at (vv));}
		       );
	auto cfa = copyCFA (function->getCFA (),map);
	auto retType  = function->getReturnType ();
	return program.addFunction (function->getSymbol ().getName(),
				    parameters,
				    retType,
				    std::move(varstack),
				    std::move(cfa));
      }

      
      
      void copyProgram (const Program& p) {
	RegReplaceMap replace_map;
	program.getCPURegs () = copyVariables (p.getCPURegs (),replace_map);
	
	for (auto f : p.getFunctions ()) {
	  copyFunction (f,replace_map);
	}
	
	for (auto f: p.getEntryPoints ()) {
	  program.addEntryPoint (f->getSymbol ().getName());
	}

	program.setInitialiser (copyInstructionStream(p.getInitialiser (),replace_map));
	
	program.getHeapLayout () = p.getHeapLayout ();
	
      }
      
      Program& program;
    };
    

    Program::Program (const Program& p) : Program(p.tfact,p.cfact) {
      Copier{*this}.copyProgram(p);
    }
  }
}
