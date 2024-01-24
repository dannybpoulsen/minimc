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

	else if constexpr (i == InstructionCode::PtrSub) {
	  return os << *content.res << " = " <<InstructionCode::PtrSub << " " << *content.ptr << " " << *content.skipsize << " " << *content.nbSkips;
	;	  
	}
      
	else if constexpr ( i == InstructionCode::ExtractValue) {
	  return os << *content.res << " = " << InstructionCode::ExtractValue << " " << *content.res->getType () << " " << *content.aggregate << " "  << *content.offset;
	}

	else if constexpr ( i == InstructionCode::Assign) {
	  return os << *content.res << " = " << *content.op1;
	}
	
	
	
	else if constexpr ( i == InstructionCode::Ret) {
	  return os << "Ret " << *content.value;
	  
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
    
    /*template<class T>
    T copyReplace (const T& t, ReplaceFunction replace) {
      if constexpr (std::is_same<TACContent, T> ()) {
	return {replace(t.res), replace(t.op1), replace(t.op2)};
      }

      else if constexpr (std::is_same<UnaryContent, T> ()) {
	return {replace(t.res), replace(t.op1)};
      }

      else if constexpr (std::is_same<BinaryContent,T> ()) {
	return {replace(t.op1), replace(t.op2)};
      }
      else if constexpr (std::is_same<ExtractContent,T> ()) {
	return {replace(t.res), replace(t.aggregate), replace (t.offset)};
      }

      else if constexpr (std::is_same<InsertContent,T> ()) {
	return {replace(t.res), replace(t.aggregate), replace (t.offset), replace (t.insertee)};
		
      }

      else  if constexpr (std::is_same<PtrAddContent,T> ()) {
	return {replace(t.res),
	        replace(t.ptr),
		replace(t.skipsize),
		replace(t.nbSkips)
	};
      }
      
      
      else if constexpr (std::is_same<NonDetContent,T> ()) {
	return {replace(t.res),replace(t.min), replace(t.max)};
      }

      else if constexpr (std::is_same<AssertAssumeContent,T> ()) {
	return {replace(t.expr)};
      }

      else if constexpr (std::is_same<LoadContent,T> ()) {
	return {replace(t.res), replace(t.addr)};
      }
      
      else if constexpr (std::is_same<StoreContent,T> ()) {
	return {replace(t.addr), replace(t.storee)};
      }

      else if constexpr (std::is_same<RetContent,T> ()) {
	return {replace(t.value)};
      }

      else if constexpr (std::is_same<CallContent,T> ()) {
	std::vector<Value_ptr> params;
	auto inserter = std::back_inserter(params);
	std::for_each (t.params.begin(),t.params.end(),[replace,&inserter](auto& p) {inserter = replace(p);}); 
	
	return {replace(t.res), replace(t.function), params};

      }

      else if constexpr (std::is_same<int,T> ()) {
	return t;
      }

      else {
	[]<bool flag = false>() {static_assert(flag, "Uncopiable Object");}();
      }
      
    }
    */

    //Copy Consructor with Replacement :-)
    Instruction::Instruction (const Instruction& oth, ReplaceFunction replace)  : internal(oth.internal) {
      internal = oth.visit([replace](auto& tc) -> Instruction_internal {
	if constexpr (hasOperands<tc.getOpcode ()>) {
	  return TInstruction<tc.getOpcode ()>(typename InstructionData<tc.getOpcode ()>::Content (tc.getOps (), replace));
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
