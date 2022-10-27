#ifndef _PATHIMPL__
#define _PATHIMPL__

#include "util/smtconstruction.hpp"
#include "util/ssamap.hpp"

#include "heap.hpp"

namespace MiniMC {
  namespace CPA {
    namespace PathFormula {

      struct VMData {
        const MiniMC::Util::SSAMap* oldSSAMap;
        MiniMC::Util::SSAMap* newSSAMap;

        const Heap* oldHeap;
        Heap* newHeap;

        SMTLib::TermBuilder* smtbuilder;
        SMTLib::Term_ptr path;
        void finalise() {}
      };

      template <MiniMC::Model::InstructionCode c>
      class NotImplemented : public MiniMC::Support::Exception {
      public:
        NotImplemented() : MiniMC::Support::Exception(MiniMC::Support::Localiser{"Instruction '%1%' not implemented for this CPA"}.format(c)) {}
      };

      struct ExecuteInstruction {
        template <MiniMC::Model::InstructionCode opc>
        static void execute(VMData& data,
                            const MiniMC::Model::Instruction& i) {
	  auto& content = i.getOps<opc> ();
          if constexpr (MiniMC::Model::InstructionData<opc>::isTAC ||
                        MiniMC::Model::InstructionData<opc>::isComparison) {
            auto& res = content.res;
            auto& left = content.op1;
            auto& right = content.op2;
            SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc>();
            auto leftTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.smtbuilder, left); //data.oldSSAMap->lookup (left.get());
            auto rightTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.smtbuilder, right);
            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(smtop, {leftTerm, rightTerm}));
          } else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate) {
	    auto& left = content.op1;
            auto& right = content.op2;
            SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc>();
            auto leftTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, left); //data.oldSSAMap->lookup (left.get());
            auto rightTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, right);
            auto conjunct = data.smtbuilder->buildTerm(smtop, {leftTerm, rightTerm});
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, conjunct});

          } else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
            auto& res = content.res;
            auto& value = content.op1;
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, value);
            data.newSSAMap->updateValue(res.get(), valTerm);
          } else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
            auto assert = content.expr;
            auto assertTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.smtbuilder, assert);
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, assertTerm});
          } else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
            auto assert = content.expr;
            auto assertTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.smtbuilder, assert);
            auto notted = data.smtbuilder->buildTerm(SMTLib::Ops::Not, {assertTerm});
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, notted});

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::NonDet) {
            auto& res = content.res;
            auto valTerm = MiniMC::Util::buildSMTValue(*data.smtbuilder, res);

            data.newSSAMap->updateValue(res.get(), valTerm);

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::BoolZExt) {
	    auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1;	    
	    
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.smtbuilder, castee);

            auto bytesize = res->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto ones = data.smtbuilder->makeBVIntConst(1, bytesize * 8);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {valTerm, ones, zeros}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::BoolSExt) {
	    auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1;	    
	    
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, castee);

            auto bytesize = res->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto ones = data.smtbuilder->makeBVIntConst(~0, bytesize * 8);
	    
            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {valTerm, ones, zeros}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {
	    auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1;            
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, castee);
            auto bytesize = res->getType()->getSize() - castee->getType()->getSize();

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::SExt, {valTerm}, {bytesize * 8}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {
	    auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1; 
	   
	    auto bytesize = res->getType()->getSize() - castee->getType()->getSize();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, castee);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ZExt, {valTerm}, {bytesize * 8}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {
	    auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1;

	    auto bytesize = res->getType()->getSize();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, castee);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::Extract, {valTerm}, {bytesize * 8 - 1, 0}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::IntToBool) {

            auto& content = i.getOps<opc> ();
            auto& res = content.res;
            auto& castee = content.op1;

	    auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, castee);

            auto bytesize = castee->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto eq = data.smtbuilder->buildTerm(SMTLib::Ops::Equal, {valTerm, zeros});
            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {eq,
                                                                                                 data.smtbuilder->makeBoolConst(false),
                                                                                                 data.smtbuilder->makeBoolConst(true)}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Alloca ||

                             opc == MiniMC::Model::InstructionCode::FindSpace) {
	    auto& content = i.getOps<opc> ();
	    auto& result = content.res;
            auto& size = content.op1;
            if (size->isRegister ()) {
              throw MiniMC::Support::Exception("Symbolic Allocation sizes not supported");
            }

            else {
              auto constant = std::static_pointer_cast<MiniMC::Model::Constant>(size);
              if (constant->isUndef()) {
                //This case should actually be ruled out earlier
                throw MiniMC::Support::Exception("Undef Allocation sizes not supported");
              } else {
                MiniMC::uint64_t allocation_size;
                std::copy(constant->getData(), constant->getData() + sizeof(MiniMC::uint64_t), reinterpret_cast<MiniMC::uint8_t*>(&allocation_size));
                MiniMC::Util::SSAMap* upd = data.newSSAMap;
                upd->updateValue(result.get(),
                                 data.newHeap->allocate(allocation_size, *data.smtbuilder));
              }
            }
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Load) {
            auto& result = content.res;
            auto& addr = content.addr;
            auto addr_term = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, addr);
            auto read_data = data.oldHeap->read(addr_term,
                                                result->getType()->getSize(),
                                                *data.smtbuilder);

            data.newSSAMap->updateValue(result.get(), read_data);
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Store) {
            
            auto addr = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, content.addr);
            auto val = MiniMC::Util::buildSMTTerm(*data.oldSSAMap,  *data.smtbuilder, content.storee);
            data.newHeap->write(val, addr, content.storee->getType()->getSize(), *data.smtbuilder);
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
            //Do nothing
          }

          /*else if constexpr (opc == MiniMC::Model::InstructionCode::Malloc) {
            //Do nothing
	    }*/

          else {
            throw NotImplemented<opc>();
          }
        }
      };

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC

#endif
