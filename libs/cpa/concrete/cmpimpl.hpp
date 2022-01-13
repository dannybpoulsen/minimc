#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "support/cmps.hpp"
#include "support/operataions.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "util/array.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {

      template <MiniMC::Model::InstructionCode opc, class T>
      MiniMC::Util::Array cmpexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        MiniMC::Util::Array res(sizeof(MiniMC::uint8_t));
        auto ll = left.template read<T>();
        auto rr = right.template read<T>();

        if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_SGT) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::SGT>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_UGT) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::UGT>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_SGE) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::SGE>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_UGE) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::UGE>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_SLT) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::SLT>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_ULT) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::ULT>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_SLE) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::SGE>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_ULE) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::ULE>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_EQ) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::EQ>(ll, rr));
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::ICMP_NEQ) {
          res.template set<MiniMC::uint8_t>(0, MiniMC::Support::Op<MiniMC::Support::CMP::NEQ>(ll, rr));
        }

        return res;
      }

      template <MiniMC::Model::InstructionCode opc>
      MiniMC::Util::Array Stepcmpexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        assert(left.getSize() == right.getSize());

        switch (left.getSize()) {
          case 1:
            return cmpexec<opc, MiniMC::uint8_t>(left, right);
          case 2:
            return cmpexec<opc, MiniMC::uint16_t>(left, right);
          case 4:
            return cmpexec<opc, MiniMC::uint32_t>(left, right);
          case 8:
            return cmpexec<opc, MiniMC::uint64_t>(left, right);
          default:
            throw MiniMC::Support::Exception(MiniMC::Support::Localiser("Unsupport size %0% for operation '%1%").format(left.getSize(), opc));
        }
      }

      template <MiniMC::Model::InstructionCode opc, class T>
      void predexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        auto ll = left.template read<T>();
        auto rr = right.template read<T>();

        if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_SGT) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::SGT>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_UGT) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::UGT>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_SGE) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::SGE>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_UGE) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::UGE> (ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_SLT) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::SLT> (ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_ULT) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::ULT> (ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_SLE) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::SLE>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_ULE) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::ULE>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_EQ) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::EQ>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }

        else if constexpr (opc == MiniMC::Model::InstructionCode::PRED_ICMP_NEQ) {
          if (!MiniMC::Support::Op<MiniMC::Support::CMP::NEQ>(ll, rr)) {
            throw MiniMC::Support::AssumeViolated();
          }
        }
      }

      template <MiniMC::Model::InstructionCode opc>
      void Steppredexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        assert(left.getSize() == right.getSize());
        switch (left.getSize()) {
          case 1:
            predexec<opc, MiniMC::uint8_t>(left, right);
            break;
          case 2:
            predexec<opc, MiniMC::uint16_t>(left, right);
            break;
          case 4:
            predexec<opc, MiniMC::uint32_t>(left, right);
            break;
          case 8:
            predexec<opc, MiniMC::uint64_t>(left, right);
            break;
          default:
            throw MiniMC::Support::Exception(MiniMC::Support::Localiser("Unsupport size %0% for operation '%1%").format(left.getSize(), opc));
        }
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
