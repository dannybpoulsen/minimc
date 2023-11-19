#include "model/checkers/structuralchecker.hpp"
#include "model/valuevisitor.hpp"

#include <unordered_set>
#include <iostream>

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      bool StructuralChecker::Check (MiniMC::Model::Program& prgm, MiniMC::Support::Messager mess) {
	mess << MiniMC::Support::TInfo {"Structural Checks"};
	for (auto& F : prgm.getFunctions ()) {
	  for (auto& e : F->getCFA ().getEdges ()) {
	    std::unordered_set<MiniMC::Model::Value_ptr> defined;
	    for (auto& instr : e->getInstructions ()) {
	      if (!instr.visit (MiniMC::Model::Overload {
		    [&defined]<typename T>(const T& a) requires (T::hasOperands())  {

		      std::vector<MiniMC::Model::Value_ptr> vals;
		      a.getUsages (std::back_inserter(vals));
		      

		      for (auto& v : vals) {
			if (!defined.count (v)) {
			  if (v->isRegister ()) {
			    auto casted_v = std::static_pointer_cast<MiniMC::Model::Register> (v);
			    if (casted_v->getRegType ()== MiniMC::Model::RegType::Meta) {
			      std::cerr << *v << "used without definition" << std::endl;
			      return false;
			    }
			  }
			}
		      }
		      
		      if constexpr (T::hasResult ()) {
			defined.insert( a.getDefines());
			return true;
			
		      }
		    
		      return true;
		    },
		      [](auto&)   {return true;}
		      
		}
		  ) ) {
		return false;
	      }
	      
	    }
	    
	  }
	}
	
	return true;
      }

      
      
    }
  }
}
