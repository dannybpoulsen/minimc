#include "minimc/model/instructions.hpp"

namespace MiniMC {
  namespace Model {
    
    template<class I>
    std::ostream& format (std::ostream& os, const I& inst) {
      constexpr auto i = I::getOpcode ();
      if constexpr (!hasOperands<I::getOpcode ()> ) {
	return os << I::getOpcode ();
      }
      else {
	
	auto& content = inst.getOps  ();
	if constexpr (isTAC_v<I> || isComparison_v<I> ) {
	  return os << *content.res << " = " << i << " " << *content.op1 << " " << *content.op2;   
	}
	
	
	else if constexpr (isUnary_v<I>) {
	  return os << *content.res << " = " << i << " " << *content.op1;
	}
	
	else if constexpr (isCast_v<I>) {
	  return os << *content.res << " = " << i << " "<< *content.res->getType () << " " <<  *content.op1;
	}
	
	else if constexpr (i == VMInstructionCode::NonDet) {
	  return os << *content.res << " = " << VMInstructionCode::NonDet << " "  << *content.res->getType () << " " << *content.min << " " << *content.max;
	}
	
      
	else if constexpr (i == VMInstructionCode::Assert ||
			   i == VMInstructionCode::Assume ) {
	  return os << i << " " << *content.expr;
	}

	else if constexpr (i == VMInstructionCode::Call) {
	  if (content.res) {
	    os << *content.res << " = ";
	  }
	  os << VMInstructionCode::Call << " " << *content.function << " ";
	  for (auto& v : content.params)
	  os << *v << " ";
	  return os;
	}

	else if constexpr ( i == VMInstructionCode::ExtractValue) {
	  return os << *content.res << " = " << VMInstructionCode::ExtractValue << " " << *content.res->getType () << " " << *content.aggregate << " "  << *content.offset;
	}

	else if constexpr ( i == VMInstructionCode::Assign) {
	  return os << *content.res << " = " << *content.op1;
	}
	
	
	
	else if constexpr ( i == VMInstructionCode::Ret) {
	  return os << "Ret " << *content.value;
	  
	}
	
	else if constexpr ( i == VMInstructionCode::InsertValue) {
	  return os << *content.res << " = " << VMInstructionCode::InsertValue << " " << *content.insertee->getType () << " " << *content.aggregate << " "  << *content.offset << " " << *content.insertee;
	}
	
	else if constexpr ( i == VMInstructionCode::Uniform) {
	  return os << VMInstructionCode::Uniform;
	}
	
	else if constexpr ( i == VMInstructionCode::Store ) {
	  return os << *content.res << " = " << VMInstructionCode::Store << "  " << *content.storeto << " " << *content.addr << " " << *content.storee;	
	}
	
	
	else {
	  []<bool b = false> () {static_assert(b && "No format known for this tyoe");} ();
	}
	
      }     
      
    }

    
    std::ostream& operator<<(std::ostream& os, const InstructionStream& str) {
      return str.output (os);
    }
    
    
    std::ostream& operator<<(std::ostream& os, const std::vector<Instruction>& str) {
      for (auto& i : str) {
        os << i << std::endl;
      }
      return os;
    }
    

    std::ostream& Instruction::output(std::ostream& os) const {
      visit ([&os](const auto& a) {format (os,a);});
      return os;
    }
    
    //Copy Consructor with Replacement :-)
    Instruction::Instruction (const Instruction& oth, ReplaceFunction replace)  : internal(oth.internal) {
      internal = oth.visit([replace](auto& tc) -> Instruction_internal {
	if constexpr (hasOperands<tc.getOpcode ()>) {
	  return TInstruction<tc.getOpcode ()>(typename VMInstructionData<tc.getOpcode ()>::Content (tc.getOps (), replace));
	}
	else {
	  return TInstruction<tc.getOpcode ()>(); 
	}
      }
	);
      
    }
      

    #include "minimc/model/instruction_constructors.inc"
    
  }
}
