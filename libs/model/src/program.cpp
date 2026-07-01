#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/variables.hpp"

#include <memory>
#include <unordered_map>
#include <algorithm>

namespace MiniMC {
  namespace Model {
    struct Copier {
      using RegReplaceMap = std::unordered_map<MiniMC::Model::Symbol,Register_ptr>;
      
      void copyVariables ( const MiniMC::Model::RegisterDescr& vars, RegReplaceMap& map,MiniMC::Model::RegisterDescr& stack, MiniMC::Model::Frame& frame) {
	
	for (auto& v : vars.getRegisters ()) {
	  auto vv = stack.addRegister (frame.makeSymbol (v.getSymbol().getName ()),v.getType ());
	  map.emplace (v.getSymbol(),vv);
	}
      }
      
      MiniMC::Model::InstructionStream copyInstructionStream (const MiniMC::Model::InstructionStream& instr,
							      const RegReplaceMap& repl
							   ) {
	auto replaceF = [&repl](auto& v) -> MiniMC::Model::Value_ptr {
	  return repl.at (v.getSymbol());
	};
        MiniMC::Model::Replacer replacer{replaceF};
        
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
	auto symbol = prgm.getRootFrame().makeSymbol (function->getSymbol().getName ());
	
	MiniMC::Model::RegisterDescr varstack{MiniMC::Model::RegType::Local};
	copyVariables (function->getRegisterDescr (),map,varstack,frame);
	std::vector<MiniMC::Model::Symbol> parameters;
	std::ranges::for_each (function->getParameters(),
			       [&map,&parameters](auto vv) {parameters.push_back (map.at (vv->getSymbol())->getSymbol());}
		       );
	auto cfa = copyCFA (function->getCFA (),map,frame);
	auto retType  = function->getReturnType ();
	return prgm.addFunction (symbol,
				    parameters,
				    retType,
				    std::move(varstack),
				    std::move(cfa),
				    function->isVarArgs (),
				    frame
				    );
      }

      
      
      
    };
    
    
    
  }
}
