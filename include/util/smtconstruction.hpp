#ifndef _SMTCONSTR__
#define _SMTCONSTR__

#include "util/ssamap.hpp"
#include "model/cfg.hpp"
#include "smt/builder.hpp"
#include "support/exceptions.hpp"

namespace MiniMC {
  namespace Util {
    template <SMTLib::Ops op>
      struct Chainer {
        Chainer(SMTLib::TermBuilder* builder) : builder(builder) {}
      //construct term op nterm
        Chainer& operator<<(SMTLib::Term_ptr nterm) {
          if (!term)
            term = nterm;
          else {
            term = builder->buildTerm(op, {term, nterm});
          }
          return *this;
        }

      //construct nterm op term 
      Chainer& operator>>(SMTLib::Term_ptr nterm) {
          if (!term)
            term = nterm;
          else {
            term = builder->buildTerm(op, {nterm,term});
          }
          return *this;
        }
      
        auto getTerm() const { return term; }

        SMTLib::Term_ptr term = nullptr;
        SMTLib::TermBuilder* builder;
      };
      
      
    
    struct PointerHelper {
        PointerHelper(SMTLib::TermBuilder* b) : builder(b) {}

        SMTLib::Term_ptr isStackConstraint(SMTLib::Term_ptr& term) {
          //Extract last eight bits;
          auto stack_seg = builder->buildTerm(SMTLib::Ops::Extract, {term}, {63, 63 - sizeof(MiniMC::seg_t) * 8});
          auto stack_id = builder->makeBVIntConst(static_cast<MiniMC::seg_t>(MiniMC::Support::PointerType::Stack), 8);
          return builder->buildTerm(SMTLib::Ops::Equal, {stack_seg, stack_id});
        }

        SMTLib::Term_ptr baseValue(SMTLib::Term_ptr& term) {
          return builder->buildTerm(SMTLib::Ops::Extract, {term}, {47, 32});
        }

        SMTLib::Term_ptr offsetValue(SMTLib::Term_ptr& term) {
          return builder->buildTerm(SMTLib::Ops::Extract, {term}, {31, 0});
        }

        SMTLib::Term_ptr makeStackPointer(MiniMC::base_t base, MiniMC::offset_t) {

          auto stack_term = builder->makeBVIntConst(static_cast<MiniMC::seg_t>(MiniMC::Support::PointerType::Stack), 8 * sizeof(MiniMC::seg_t));
          auto zero_term = builder->makeBVIntConst(0, 8 * sizeof(MiniMC::int8_t));

          auto base_term = builder->makeBVIntConst(base, 8 * sizeof(MiniMC::base_t));
          auto offset_term = builder->makeBVIntConst(base, 8 * sizeof(MiniMC::offset_t));

          return (Chainer<SMTLib::Ops::Concat>{builder} << stack_term << zero_term << base_term << offset_term).getTerm();
        }
      
      SMTLib::Term_ptr makeHeapPointer(MiniMC::base_t base, MiniMC::offset_t offset) {
	
	auto stack_term = builder->makeBVIntConst(static_cast<MiniMC::seg_t>(MiniMC::Support::PointerType::Heap), 8 * sizeof(MiniMC::seg_t));
	auto zero_term = builder->makeBVIntConst(0, 8 * sizeof(MiniMC::int8_t));
	
	auto base_term = builder->makeBVIntConst(base, 8 * sizeof(MiniMC::base_t));
	auto offset_term = builder->makeBVIntConst(offset, 8 * sizeof(MiniMC::offset_t));
	
	return (Chainer<SMTLib::Ops::Concat>{builder} << stack_term << zero_term << base_term << offset_term).getTerm();
      }

      SMTLib::Term_ptr makePointer(const MiniMC::pointer_t& pointer ) {
	auto seg_term = builder->makeBVIntConst(pointer.segment, 8 * sizeof(pointer.segment));
	auto zero_term = builder->makeBVIntConst(pointer.zero, 8 * sizeof(pointer.zero));
	auto base_term = builder->makeBVIntConst(pointer.base, 8 * sizeof(pointer.base));
	auto offset_term = builder->makeBVIntConst(pointer.offset, 8 * sizeof(pointer.offset));
	return (Chainer<SMTLib::Ops::Concat>{builder} << seg_term << zero_term << base_term << offset_term).getTerm();
      }
      
      SMTLib::TermBuilder* builder;
    };

    template <MiniMC::Model::InstructionCode Inst>
    constexpr SMTLib::Ops convertToSMTOp() {
      if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SGT) {
        return SMTLib::Ops::BVSGt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_UGT) {
        return SMTLib::Ops::BVUGt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SGE) {
        return SMTLib::Ops::BVSGEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_UGE) {
        return SMTLib::Ops::BVUGEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SLT) {
        return SMTLib::Ops::BVSLt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_ULT) {
        return SMTLib::Ops::BVULt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_SLE) {
        return SMTLib::Ops::BVSLEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_ULE) {
        return SMTLib::Ops::BVULEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_EQ) {
        return SMTLib::Ops::Equal;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ICMP_NEQ) {
        return SMTLib::Ops::NotEqual;
      }
      if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SGT) {
        return SMTLib::Ops::BVSGt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_UGT) {
        return SMTLib::Ops::BVUGt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SGE) {
        return SMTLib::Ops::BVSGEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_UGE) {
        return SMTLib::Ops::BVUGEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SLT) {
        return SMTLib::Ops::BVSLt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_ULT) {
        return SMTLib::Ops::BVULt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_SLE) {
        return SMTLib::Ops::BVSLEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_ULE) {
        return SMTLib::Ops::BVULEq;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_EQ) {
        return SMTLib::Ops::Equal;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::PRED_ICMP_NEQ) {
        return SMTLib::Ops::NotEqual;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Add) {
        return SMTLib::Ops::BVAdd;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Sub) {
        return SMTLib::Ops::BVSub;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Mul) {
        return SMTLib::Ops::BVMul;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::UDiv) {
        return SMTLib::Ops::BVUDiv;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::SDiv) {
        return SMTLib::Ops::BVSDiv;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Shl) {
        return SMTLib::Ops::BVLShl;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::LShr) {
        return SMTLib::Ops::BVLShr;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::AShr) {
        return SMTLib::Ops::BVAShr;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::And) {
        return SMTLib::Ops::BVAnd;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Or) {
        return SMTLib::Ops::Or;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::Xor) {
        return SMTLib::Ops::Xor;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::ZExt) {
        return SMTLib::Ops::ZExt;
      } else if constexpr (Inst == MiniMC::Model::InstructionCode::SExt) {
        return SMTLib::Ops::SExt;
      } else
        throw MiniMC::Support::Exception("Can't map instructioncode to SMT");
    }

    inline SMTLib::Term_ptr buildSMTConstant(SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
      auto constant = std::static_pointer_cast<MiniMC::Model::Constant>(ptr);
      auto type = ptr->getType();

      if (constant->isInteger()) {
	auto create = [&]<typename T>(const MiniMC::uint8_t* data) {
          return builder.makeBVIntConst(*reinterpret_cast<const T*>(data), sizeof(T) * 8);
        };
        switch (type->getSize()) {
          case 1:
            return create.operator()<MiniMC::uint8_t>(constant->getData());
          case 2:
            return create.operator()<MiniMC::uint16_t>(constant->getData());
          case 4:
            return create.operator()<MiniMC::uint32_t>(constant->getData());
          case 8:
            return create.operator()<MiniMC::uint64_t>(constant->getData());
        }
        throw MiniMC::Support::Exception("Bug");
      }
      
      else if (constant->isUndef()) {
	static std::size_t nb = 0;
        auto sort = builder.makeBVSort(type->getSize() * 8);
        return builder.makeVar(sort, MiniMC::Support::Localiser("SMT-Undef-%1%").format(++nb));
      }

      else if (type->getTypeID () == MiniMC::Model::TypeID::Pointer) {
	assert(constant->isBinaryBlobConstant ());
	auto blob = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant> (ptr);
	auto pointer = blob->template getValue<MiniMC::pointer_t> ();
	return PointerHelper{&builder}.makePointer (pointer);
      }
      
      else {
	throw MiniMC::Support::Exception("Unsupported Constant");
      }
    }

    inline SMTLib::Term_ptr buildSMTValue(SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
      static std::size_t nb = 0;
      if (ptr->isConstant()) {
        return buildSMTConstant(builder, ptr);
      } else {
        auto var = std::static_pointer_cast<MiniMC::Model::Register>(ptr);
        auto type = ptr->getType();
        SMTLib::Sort_ptr sort = nullptr;
        if (type->getTypeID() == MiniMC::Model::TypeID::Integer) {
          switch (type->getSize()) {
            case 1:
              sort = builder.makeBVSort(8);
              break;
            case 2:
              sort = builder.makeBVSort(16);
              break;
            case 4:
              sort = builder.makeBVSort(32);
              break;
            case 8:
              sort = builder.makeBVSort(64);
              break;
          }

        } else if (type->getTypeID() == MiniMC::Model::TypeID::Bool) {
          sort = builder.makeSort(SMTLib::SortKind::Bool, {});
        } else if (type->getTypeID() == MiniMC::Model::TypeID::Pointer) {
          sort = builder.makeBVSort(64);
        }

        if (sort)
          return builder.makeVar(sort, MiniMC::Support::Localiser("SMT-%1%").format(++nb));
      }
      throw MiniMC::Support::Exception("Error");
    }

    /*    SMTLib::Term_ptr buildSMTTerm(const SSAMap& map,  SMTLib::TermBuilder& builder, const MiniMC::Model::Value_ptr& ptr) {
      if (!ptr->isConstant()) {
          return map.lookup(ptr.get());
      } else {
        return buildSMTConstant(builder, ptr);
      }
    }
    */
  } // namespace Util
} // namespace MiniMC

#endif
