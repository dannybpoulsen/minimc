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
      
      void copyVariables ( const MiniMC::Model::RegisterDescr& vars, RegReplaceMap& map,MiniMC::Model::RegisterDescr& stack, MiniMC::Model::Frame& frame) {
	
	for (auto& v : vars.getRegisters ()) {
	  map.emplace (v,stack.addRegister (frame.makeSymbol (v->getSymbol().getName ()),v->getType ()));
	}
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
		    const RegReplaceMap& vars,
		    Frame& frame
		    ) {
	MiniMC::Model::CFA ncfa;
	std::unordered_map<Location_ptr, Location_ptr> locMap;

	for (auto& loc : cfa.getLocations ()) {
	  locMap.emplace (loc,ncfa.makeLocation (frame.makeSymbol (loc->getSymbol ().getName ()), loc->getInfo ()));
	}

	ncfa.setInitial (locMap.at(cfa.getInitialLocation ()));
	
	for (auto& e : cfa.getEdges ()) {
	  auto& instrs = e->getInstructions();
	  
	  auto nedge = ncfa.makeEdge (locMap.at (e->getFrom ()), locMap.at (e->getTo ()),copyInstructionStream (instrs,vars));

	  
	}
	
	return ncfa;
	
      }

      auto copyFunction (const Function_ptr& function,RegReplaceMap map,Program& prgm) {
	auto frame = prgm.getRootFrame().create (function->getSymbol().getName ());
	MiniMC::Model::RegisterDescr varstack{MiniMC::Model::RegType::Local};
	copyVariables (function->getRegisterDescr (),map,varstack,frame);
	std::vector<Register_ptr> parameters;
	std::for_each (function->getParameters().begin (),
		       function->getParameters ().end(),
		       [&map,&parameters](auto& vv) {parameters.push_back (map.at (vv));}
		       );
	auto cfa = copyCFA (function->getCFA (),map,frame);
	auto retType  = function->getReturnType ();
	return prgm.addFunction (function->getSymbol ().getName(),
				    parameters,
				    retType,
				    std::move(varstack),
				    std::move(cfa),
				    function->isVarArgs (),
				    frame
				    );
      }

      
      
      
    };
    

    Program::Program (const Program& p) : Program(p.tfact,p.cfact) {
      Copier copier;
      
      Copier::RegReplaceMap replace_map;
      copier.copyVariables (p.getCPURegs (),replace_map,cpu_regs,getRootFrame());
      
      for (auto f : p.getFunctions ()) {
	copier.copyFunction (f,replace_map,*this);
      }
      
      for (auto f: p.getEntryPoints ()) {
	addEntryPoint (f->getSymbol ().getName());
      }
      
      setInitialiser (copier.copyInstructionStream(p.getInitialiser (),replace_map));
      
      getHeapLayout () = p.getHeapLayout ();
      
      
    }
  }
}
