#include "model/instructions.hpp"

#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "support/rightshifts.hpp"
#include "util/array.hpp"
#include "util/semmap.hpp"

namespace MiniMC {
  namespace CPA {
    namespace Concrete {

      
      
      template <MiniMC::Model::InstructionCode opc, class T>
      MiniMC::Util::Array tacexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        MiniMC::Util::Array res(sizeof(T));
        auto l = left.template read<T>();
        auto r = right.template read<T>();

	res.template set<T>(0, MiniMC::Support::Op<MiniMC::Util::InstructionCodeToOperation<opc> ()> (l,r));
	return res;
      }

      template <MiniMC::Model::InstructionCode opc>
      MiniMC::Util::Array Steptacexec(const MiniMC::Util::Array& left, const MiniMC::Util::Array& right) {
        assert(left.getSize() == right.getSize());
        switch (left.getSize()) {
          case 1:
            return tacexec<opc, MiniMC::uint8_t>(left, right);
          case 2:
            return tacexec<opc, MiniMC::uint16_t>(left, right);
          case 4:
            return tacexec<opc, MiniMC::uint32_t>(left, right);
          case 8:
            return tacexec<opc, MiniMC::uint64_t>(left, right);
          default:
            throw MiniMC::Support::Exception(MiniMC::Support::Localiser("Unsupport size %0% for operation '%1%").format(left.getSize(), opc));
        }
      }

    } // namespace Concrete
  }   // namespace CPA
} // namespace MiniMC
