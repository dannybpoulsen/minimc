#include "model/instructions.hpp"

namespace MiniMC {
  namespace Model {

    template<InstructionCode i>
    std::ostream& format (std::ostream& os, const Instruction& inst) {
      auto& content = inst.getOps<i> ();
      if constexpr (InstructionData<i>::isTAC || InstructionData<i>::isComparison) {
	return os << *content.res << " = " << i << " " << *content.op1 << " " << *content.op1;   
      }

      else if constexpr (InstructionData<i>::isPredicate) {
	return os << i << " " << *content.op1 << " " << *content.op2;
      }

      else if constexpr (InstructionData<i>::isUnary) {
	return os << *content.res << " = " << i << " " << *content.op1;
      }

      else if constexpr (InstructionData<i>::isCast) {
	return os << *content.res << " = " << i << *content.op1;
      }

      else if constexpr (i == InstructionCode::Alloca) {
	return os << InstructionCode::Alloca << " (" << *content.res << ", " << *content.op1 << " )";  
      }

      else if constexpr (i == InstructionCode::ExtendObj) {
	return os << InstructionCode::ExtendObj << " (" << *content.res << ", " << *content.object << " by " << *content.size << " )";
      
      }

      else if constexpr (i == InstructionCode::Malloc) {
	return os << InstructionCode::Malloc << " (" << *content.object << ", " << *content.size << " )";
      }

      else if constexpr ( i == InstructionCode::Free) {
	return os << InstructionCode::Free << " (" << *content.object << " )";
      }

      else if constexpr (i == InstructionCode::FindSpace) {
	return os << *content.res << " = " << InstructionCode::FindSpace << *content.op1;
      
      }

      else if constexpr (i == InstructionCode::Skip) {
	return os << InstructionCode::Skip;
      }

      else if constexpr (i == InstructionCode::NonDet) {
	return os << *content.res << "=" << InstructionCode::NonDet << "(" << *content.min << ", " << *content.max << ")";
      }

      else if constexpr (i == InstructionCode::StackSave) {
	return os << *content.res << "=" << InstructionCode::StackSave;
      }
      
      else if constexpr (i == InstructionCode::StackRestore) {
	return os << InstructionCode::StackRestore << "( " << *content.stackobject << " )";
      }

      else if constexpr (i == InstructionCode::MemCpy) {
	return os << InstructionCode::MemCpy;  
      }

      else if constexpr (i == InstructionCode::Assert ||
			 i == InstructionCode::Assume ||
			 i == InstructionCode::NegAssume) {
	return os << i << "(" << *content.expr << ")";
      }

      else if constexpr (i == InstructionCode::Call) {
	return os << InstructionCode::Call;	  
      }

      else if constexpr (i == InstructionCode::PtrAdd) {
	return os << InstructionCode::PtrAdd;	  
      }

      else if constexpr ( i == InstructionCode::ExtractValue) {
	return os << InstructionCode::ExtractValue;
      }

      else if constexpr ( i == InstructionCode::Assign) {
	return os << *content.res << " = " << *content.op1;
      }

      else if constexpr ( i == InstructionCode::RetVoid) {
	return os << "RetVoid";	
      }

      else if constexpr ( i == InstructionCode::Ret) {
	return os << "Ret " << *(inst.getOps<InstructionCode::Ret> ().value);

      }

      else if constexpr ( i == InstructionCode::InsertValue) {
	return os << InstructionCode::InsertValue;	
      }

      else if constexpr ( i == InstructionCode::Uniform) {
	return os << InstructionCode::InsertValue;
      }

      else if constexpr ( i == InstructionCode::Store) {
	return os << InstructionCode::Store << std::endl;	
      }

      else if constexpr ( i == InstructionCode::Load) {
		return os << InstructionCode::Load;
      }

      else {
	[]<bool b = false> () {static_assert(b && "No format known for this tyoe");} ();
      }

	     
      
    }

    
    
    
    std::ostream& operator<<(std::ostream& os, const instructionstream& str) {
      for (auto& i : str) {
        os << i << std::endl;
      }
      return os;
    }
    

    std::ostream& Instruction::output(std::ostream& os) const {
      switch (getOpcode()) {
#define X(OP)                                                 \
	case InstructionCode::OP:			      \
	  return format<InstructionCode::OP>(os, *this);      \
	  break;
	OPERATIONS
#undef X
	  }
      throw MiniMC::Support::Exception ("Not an instruction");
    }
  }
}
