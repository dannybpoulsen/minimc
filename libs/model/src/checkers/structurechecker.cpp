#include "model/checkers/structuralchecker.hpp"
#include "model/valuevisitor.hpp"

#include <unordered_set>
#include <iostream>

namespace MiniMC {
  namespace Model {
    namespace Checkers {

      class StructureError : public MiniMC::Support::ErrorMessage {
      public:
	StructureError (MiniMC::Model::Instruction instr) : instr(std::move(instr)) {}
	
      protected:
	auto& getInstr () const {return instr;}
	private:
	MiniMC::Model::Instruction instr;
	
      };

      class UsedWithoutDefinition : public StructureError{
      public:
	UsedWithoutDefinition (MiniMC::Model::Instruction instr, MiniMC::Model::Value_ptr l1,MiniMC::Model::Edge_ptr edge) : StructureError (std::move(instr)),l1(l1),edge(edge) {}
	virtual std::ostream& to_string (std::ostream& os) const {
	  return os << loc.format (getInstr (),*l1,*edge);
	}
	
      private:
	MiniMC::Support::Localiser loc{"For Instruction '%1%': '%2%' is used without a definition before on edge '%3%'"};
	MiniMC::Model::Value_ptr l1;
	MiniMC::Model::Edge_ptr edge;
      };
      
      bool StructuralChecker::Check (MiniMC::Model::Program& prgm, MiniMC::Support::Messager mess) {
	for (auto& F : prgm.getFunctions ()) {
	  for (auto& e : F->getCFA ().getEdges ()) {
	    std::unordered_set<MiniMC::Model::Value_ptr> defined;
	    for (auto& instr : e->getInstructions ()) {
	      if (!instr.visit (MiniMC::Model::Overload {
		    [&defined,&instr,e,&mess]<typename T>(const T& a) requires (T::hasOperands())  {

		      std::vector<MiniMC::Model::Value_ptr> vals;
		      a.getUsages (std::back_inserter(vals));
		      

		      for (auto& v : vals) {
			if (!defined.count (v)) {
			  if (v->isRegister ()) {
			    auto casted_v = std::static_pointer_cast<MiniMC::Model::Register> (v);
			    if (casted_v->getRegType ()== MiniMC::Model::RegType::Meta) {
			      mess << UsedWithoutDefinition (instr,v,e);
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
