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

        const MiniMC::Util::SSAMap* oldGSSAMap;
        MiniMC::Util::SSAMap* newGSSAMap;

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

          if constexpr (MiniMC::Model::InstructionData<opc>::isTAC ||
                        MiniMC::Model::InstructionData<opc>::isComparison) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto& res = helper.getResult();
            auto& left = helper.getLeftOp();
            auto& right = helper.getRightOp();
            SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc>();
            auto leftTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, left); //data.oldSSAMap->lookup (left.get());
            auto rightTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, right);
            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(smtop, {leftTerm, rightTerm}));
          } else if constexpr (MiniMC::Model::InstructionData<opc>::isPredicate) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto& left = helper.getLeftOp();
            auto& right = helper.getRightOp();
            SMTLib::Ops smtop = MiniMC::Util::convertToSMTOp<opc>();
            auto leftTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, left); //data.oldSSAMap->lookup (left.get());
            auto rightTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, right);
            auto conjunct = data.smtbuilder->buildTerm(smtop, {leftTerm, rightTerm});
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, conjunct});

          } else if constexpr (opc == MiniMC::Model::InstructionCode::Assign) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto& res = helper.getResult();
            auto& value = helper.getValue();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, value);
            data.newSSAMap->updateValue(res.get(), valTerm);
          } else if constexpr (opc == MiniMC::Model::InstructionCode::Assume) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto assert = helper.getAssert();
            auto assertTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, assert);
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, assertTerm});
          } else if constexpr (opc == MiniMC::Model::InstructionCode::NegAssume) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto assert = helper.getAssert();
            auto assertTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, assert);
            auto notted = data.smtbuilder->buildTerm(SMTLib::Ops::Not, {assertTerm});
            data.path = data.smtbuilder->buildTerm(SMTLib::Ops::And, {data.path, notted});

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::NonDet) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto& res = helper.getResult();
            auto valTerm = MiniMC::Util::buildSMTValue(*data.smtbuilder, res);

            data.newSSAMap->updateValue(res.get(), valTerm);

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::BoolZExt) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);

            auto bytesize = res->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto ones = data.smtbuilder->makeBVIntConst(1, bytesize * 8);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {valTerm, ones, zeros}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::BoolSExt) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);

            auto bytesize = res->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto ones = data.smtbuilder->makeBVIntConst(~0, bytesize * 8);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {valTerm, ones, zeros}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::SExt) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);
            auto bytesize = res->getType()->getSize() - castee->getType()->getSize();

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::SExt, {valTerm}, {bytesize * 8}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::ZExt) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto bytesize = res->getType()->getSize() - castee->getType()->getSize();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ZExt, {valTerm}, {bytesize * 8}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Trunc) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto bytesize = res->getType()->getSize();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);

            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::Extract, {valTerm}, {bytesize * 8 - 1, 0}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::IntToBool) {

            MiniMC::Model::InstHelper<opc> helper(i);

            auto& res = helper.getResult();
            auto& castee = helper.getCastee();
            auto valTerm = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, castee);

            auto bytesize = castee->getType()->getSize();
            auto zeros = data.smtbuilder->makeBVIntConst(0, bytesize * 8);
            auto eq = data.smtbuilder->buildTerm(SMTLib::Ops::Equal, {valTerm, zeros});
            data.newSSAMap->updateValue(res.get(), data.smtbuilder->buildTerm(SMTLib::Ops::ITE, {eq,
                                                                                                 data.smtbuilder->makeBoolConst(false),
                                                                                                 data.smtbuilder->makeBoolConst(true)}));

          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Alloca ||

                             opc == MiniMC::Model::InstructionCode::FindSpace) {
            MiniMC::Model::InstHelper<opc> helper(i);
            auto& result = helper.getResult();
            auto& size = helper.getSize();
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
                if (result->isGlobal()) {
                  upd = data.newGSSAMap;
                }
                upd->updateValue(result.get(),
                                 data.newHeap->allocate(allocation_size, *data.smtbuilder));
              }
            }
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Load) {
            MiniMC::Model::InstHelper<opc> helper(i);

            auto& result = helper.getResult();
            auto& addr = helper.getAddress();
            auto addr_term = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, addr);
            auto read_data = data.oldHeap->read(addr_term,
                                                result->getType()->getSize(),
                                                *data.smtbuilder);

            data.newSSAMap->updateValue(result.get(), read_data);
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Store) {
            MiniMC::Model::InstHelper<opc> helper(i);

            auto addr = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, helper.getAddress());
            auto val = MiniMC::Util::buildSMTTerm(*data.oldSSAMap, *data.oldGSSAMap, *data.smtbuilder, helper.getValue());
            data.newHeap->write(val, addr, helper.getValue()->getType()->getSize(), *data.smtbuilder);
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Skip) {
            //Do nothing
          }

          else if constexpr (opc == MiniMC::Model::InstructionCode::Malloc) {
            //Do nothing
          }

          else {
            throw NotImplemented<opc>();
          }
        }
      };

    } // namespace PathFormula
  }   // namespace CPA
} // namespace MiniMC

#endif
