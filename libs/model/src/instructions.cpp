#include "model/instructions.hpp"

namespace MiniMC {
  namespace Model {

    template<InstructionCode i>
    std::ostream& format (std::ostream& os, const Instruction& inst) {
      auto& content = inst.getOps<i> ();
      if constexpr (InstructionData<i>::isTAC || InstructionData<i>::isComparison || i == MiniMC::Model::InstructionCode::PtrEq) {
	return os << *content.res << " = " << i << " " << *content.op1 << " " << *content.op2;   
      }

      else if constexpr (InstructionData<i>::isPredicate) {
	return os << i << " " << *content.op1 << " " << *content.op2;
      }

      else if constexpr (InstructionData<i>::isUnary) {
	return os << *content.res << " = " << i << " " << *content.op1;
      }

      else if constexpr (InstructionData<i>::isCast) {
	return os << *content.res << " = " << i << " "<< *content.res->getType () << " " <<  *content.op1;
      }
      
      else if constexpr (i == InstructionCode::Skip) {
	return os << InstructionCode::Skip;
      }

      else if constexpr (i == InstructionCode::NonDet) {
	return os << *content.res << " = " << InstructionCode::NonDet << " "  << *content.res->getType () << " " << *content.min << " " << *content.max;
      }

      
      else if constexpr (i == InstructionCode::Assert ||
			 i == InstructionCode::Assume ||
			 i == InstructionCode::NegAssume) {
	return os << i << " " << *content.expr;
      }

      else if constexpr (i == InstructionCode::Call) {
	if (content.res) {
	  os << *content.res << " = ";
	}
	os << InstructionCode::Call << " " << *content.function << " ";
	for (auto& v : content.params)
	  os << *v << " ";
	return os;
      }

      else if constexpr (i == InstructionCode::PtrAdd) {
	return os << *content.res << " = " <<InstructionCode::PtrAdd  << " " << *content.ptr << " " << *content.skipsize << " " << *content.nbSkips;

	  ;	  
      }

      else if constexpr ( i == InstructionCode::ExtractValue) {
	return os << *content.res << " = " << InstructionCode::ExtractValue << " " << *content.res->getType () << " " << *content.aggregate << " "  << *content.offset;
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
	return os << *content.res << " = " << InstructionCode::InsertValue << " " << *content.insertee->getType () << " " << *content.aggregate << " "  << *content.offset << " " << *content.insertee;
      }

      else if constexpr ( i == InstructionCode::Uniform) {
	return os << InstructionCode::Uniform;
      }

      else if constexpr ( i == InstructionCode::Store ) {
	return os << InstructionCode::Store << "  " << *content.addr << " " << *content.storee;	
      }

      else if constexpr ( i == InstructionCode::Load ) {
	return os << *content.res << " = " << InstructionCode::Load << " " << *content.res->getType () <<" " << *content.addr;
      }

      else {
	[]<bool b = false> () {static_assert(b && "No format known for this tyoe");} ();
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
