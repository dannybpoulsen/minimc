#ifndef _MODEL_UTISL__
#define _MODEL_UTISL__

#include <unordered_map>

#include "model/variables.hpp"
#include "model/instructions.hpp"
#include "model/cfg.hpp"


namespace MiniMC {
  namespace Model {
    template<MiniMC::Model::InstructionCode c>
    MiniMC::Model::Value_ptr definesValue (const MiniMC::Model::Instruction& instr) {
      assert(instr.getOpcode () == c);
      if constexpr (MiniMC::Model::InstructionData<c>::hasResVar) {
	return MiniMC::Model::InstHelper<c> (instr).getResult ();
      }
      return nullptr;
    }
    
    template<MiniMC::Model::InstructionCode c,class Iterator>
    void findUsesOfInstr (const MiniMC::Model::Instruction& instr, Iterator iter) {
      assert(instr.getOpcode () == c);
      MiniMC::Model::InstHelper<c> helper (instr);
	
      if constexpr (MiniMC::Model::InstructionData<c>::isTAC ||
		    MiniMC::Model::InstructionData<c>::isComparison
		    ) {
	  iter = helper.getLeftOp ();
	  iter = helper.getRightOp ();
	}

      else if constexpr (MiniMC::Model::InstructionData<c>::isCast) {
	  iter = helper.getCastee ();
	}

      else if constexpr (c == MiniMC::Model::InstructionCode::Load) {
	  iter = helper.getAddress ();
	}

      else if constexpr (c == MiniMC::Model::InstructionCode::Store) {
	  iter = helper.getAddress ();
	  iter = helper.getValue ();
	}

      else if constexpr (c == MiniMC::Model::InstructionCode::Alloca) {
	  iter = helper.getSize();
	}

      else if constexpr (c == MiniMC::Model::InstructionCode::FindSpace) {
	  iter = helper.getSize();
	}
      
      else if constexpr (c == MiniMC::Model::InstructionCode::Malloc) {
	  iter = helper.getSize();
	}
      
      else if constexpr (c == MiniMC::Model::InstructionCode::Free) {
	   iter = helper.getPointer();
	}
      
       else if constexpr (c == MiniMC::Model::InstructionCode::ExtractValue) {
	   iter = helper.getAggregate();
	   iter = helper.getOffset();
	}

       else if constexpr (c == MiniMC::Model::InstructionCode::InsertValue) {
	   iter = helper.getAggregate();
	   iter = helper.getOffset();
	   iter = helper.getInsertee();
	   
	 }

       else if constexpr (c == MiniMC::Model::InstructionCode::InsertValueFromConst) {
	   iter = helper.getAggregate ();
	   iter = helper.getnsertee ();
	   for (size_t i = 0; i < helper.nbOps (); i++) {
	     iter = helper.getOp (i);
	   }
	 }

      else if constexpr (c == MiniMC::Model::InstructionCode::PtrAdd) {
	  iter = helper.getValue ();
	  iter = helper.getAddress ();
	  iter = helper.getSKipSize ();
	}

       else if constexpr (c == MiniMC::Model::InstructionCode::PtrEq) {
	   
	 }

       else if constexpr (c == MiniMC::Model::InstructionCode::Call) {
	   iter = helper.getFunctionPtr ();
	   for (size_t i = 0; i < helper.nbParams (); i++) {
	     iter = helper.getParam (i);
	   }

		   
	  
	 }
      
       else if constexpr (c == MiniMC::Model::InstructionCode::Assign) {
	   iter = helper.getValue ();
	   
	 }

       else if constexpr (c == MiniMC::Model::InstructionCode::Ret) {
	   iter = helper.getValue ();
	    
	 }
       else if constexpr (c == MiniMC::Model::InstructionCode::RetVoid) {
	 }
       else if constexpr (c == MiniMC::Model::InstructionCode::NonDet) {
	   
	 }
      
       else if constexpr (c == MiniMC::Model::InstructionCode::Assert) {
	   iter = helper.getAssert ();
	   
	 }

       else if constexpr (c == MiniMC::Model::InstructionCode::Assume) {
	   iter = helper.getAssert ();
	 }
      

      else if constexpr (c == MiniMC::Model::InstructionCode::NegAssume) {
	  iter = helper.getAssert ();
	    
	}
      
      else if constexpr (c == MiniMC::Model::InstructionCode::StackRestore) {
	  iter = helper.getValue ();
	  
	}
      else if constexpr (c == MiniMC::Model::InstructionCode::StackSave) {
	  
	}
      else if constexpr (c == MiniMC::Model::InstructionCode::MemCpy) {
	  iter = helper.getSource();
	  iter = helper.getTarget ();
	  iter = helper.getSize();
	}

      else if constexpr (c == MiniMC::Model::InstructionCode::Uniform) {
	  iter = helper.getMin ();
	  iter = helper.getMax ();
	  
	}
      
    }
    
    inline MiniMC::Model::Value_ptr valueDefinedBy (const MiniMC::Model::Instruction& instr) {
      switch (instr.getOpcode ()) {
#define X(CODE)								\
	case MiniMC::Model::InstructionCode::CODE:			\
	  return definesValue<MiniMC::Model::InstructionCode::CODE> (instr);
	OPERATIONS
#undef X
	  
      }
      return nullptr;
    }
    
    template<class Iterator>
    MiniMC::Model::Value_ptr valuesUsedBy (const MiniMC::Model::Instruction& instr,Iterator iter) {
      switch (instr.getOpcode ()) {
#define X(CODE)								\
	case MiniMC::Model::InstructionCode::CODE:			\
	  return findUsesOfInstr<MiniMC::Model::InstructionCode::CODE,Iterator> (instr,iter);
	OPERATIONS
#undef X
	  }
    }

        
    
  }
}

#endif
