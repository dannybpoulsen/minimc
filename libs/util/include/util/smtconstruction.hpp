#ifndef _SMTCONSTR__
#define _SMTCONSTR__

#include "smt/builder.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Util {
	template<MiniMC::Model::InstructionCode Inst>
	constexpr SMTLib::Ops convertToSMTOp () {
	  if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SGT ) {
			return SMTLib::Ops::BVSGt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_UGT ) {
		  return SMTLib::Ops::BVUGt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SGE ) {
		  return SMTLib::Ops::BVSGEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_UGE ) {
		  return SMTLib::Ops::BVUGEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SLT ) {
		  return SMTLib::Ops::BVSLt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_ULT ) {
		  return SMTLib::Ops::BVULt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SLE ) {
		  return SMTLib::Ops::BVSLEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_ULE ) {
		  return SMTLib::Ops::BVULEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_EQ ) {
		  return SMTLib::Ops::Equal;
	  }
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_NEQ ) {
	    return SMTLib::Ops::NotEqual;
	  }
	  if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SGT ) {
	    return SMTLib::Ops::BVSGt;
	  }
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_UGT ) {
		  return SMTLib::Ops::BVUGt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SGE ) {
		  return SMTLib::Ops::BVSGEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_UGE ) {
		  return SMTLib::Ops::BVUGEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SLT ) {
		  return SMTLib::Ops::BVSLt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_ULT ) {
		  return SMTLib::Ops::BVULt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SLE ) {
		  return SMTLib::Ops::BVSLEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_ULE ) {
		  return SMTLib::Ops::BVULEq;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_EQ ) {
		  return SMTLib::Ops::Equal;
	  }
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_NEQ ) {
		    return SMTLib::Ops::NotEqual;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Add) {
		  return SMTLib::Ops::BVAdd;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Sub) {
		  return SMTLib::Ops::BVSub;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Mul) {
		  return SMTLib::Ops::BVMul;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::UDiv) {
		  return SMTLib::Ops::BVUDiv;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::SDiv) {
		  return SMTLib::Ops::BVSDiv;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Shl) {
		  return SMTLib::Ops::BVLShl;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::LShr) {
		  return SMTLib::Ops::BVLShr;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::AShr) {
		  return SMTLib::Ops::BVAShr;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::And) {
		  return SMTLib::Ops::BVAnd;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Or) {
		  return SMTLib::Ops::Or;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::Xor) {
		  return SMTLib::Ops::Xor;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::ZExt) {
		  return SMTLib::Ops::ZExt;
		}
	  else if constexpr (Inst == MiniMC::Model::InstructionCode::SExt) {
		  return SMTLib::Ops::SExt;
		}
	  else 
		assert(false && "Can't map instructioncode to SMT");
	}
	
	inline SMTLib::Term_ptr buildSMTConstant (SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
	  auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (ptr);
	  if (constant->isInteger ()) {
		auto create = [&]<typename T> (const MiniMC::uint8_t* data) {
						  return builder.makeBVIntConst (*reinterpret_cast<const T*> (data),sizeof(T)*8);
						};
		auto type = ptr->getType ();
		switch (type->getSize ()) {
		case 1:
		  return create. operator()<MiniMC::uint8_t> (constant->getData()); 
		case 2:
		  return create. operator()<MiniMC::uint16_t> (constant->getData());
		case 4:
		  return create.operator()<MiniMC::uint32_t> (constant->getData());
		case 8:
		  return create.operator()<MiniMC::uint64_t> (constant->getData());
		}
		throw MiniMC::Support::Exception ("Bug");
	  }
	  
	}

	SMTLib::Term_ptr buildSMTValue (SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
	  if (ptr->isConstant()) {
		return buildSMTConstant (builder,ptr);
	  }
	  else {
		auto var = std::static_pointer_cast<MiniMC::Model::Variable> (ptr);
		auto type = ptr->getType ();
		SMTLib::Sort_ptr sort = nullptr;
		if (type->getTypeID () == MiniMC::Model::TypeID::Integer) {
		  switch (type->getSize ()) {
		  case 1:
			sort = builder.makeBVSort (8);
			break;
		  case 2:
			sort = builder.makeBVSort (16);
			break;
		  case 4:
			sort = builder.makeBVSort (32);
			break;
		  case 8:
			sort = builder.makeBVSort (64);
			break;
		  }
		  
		  
		}
		else if (type->getTypeID () == MiniMC::Model::TypeID::Bool) {
		  sort = builder.makeSort (SMTLib::SortKind::Bool,{});
		}
		if (sort)
		  return builder.makeVar (sort,var->getName ());
	  }
	  throw MiniMC::Support::Exception ("Error");
	}
	
	SMTLib::Term_ptr buildSMTTerm (const SSAMap& map, SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
	  if (!ptr->isConstant ()) {
		return map.lookup (ptr.get());
	  }
	  else {
		return buildSMTConstant (builder,ptr);
	  } 
	   
	}
	
  }
}


#endif
