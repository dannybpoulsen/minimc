#include "model/valuevisitor.hpp"
#include "support/casts.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include "vm/concrete/concrete.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      ConcreteVMVal ValueLookup::lookupValue(const MiniMC::Model::Value_ptr& v) const {

        return MiniMC::Model::visitValue(
            MiniMC::Model::Overload{
                [](const MiniMC::Model::I8Integer& val) -> ConcreteVMVal { return TValue<MiniMC::uint8_t>{val.getValue()}; },
                [](const MiniMC::Model::I16Integer& val) -> ConcreteVMVal { return TValue<MiniMC::uint16_t>{val.getValue()}; },
                [](const MiniMC::Model::I32Integer& val) -> ConcreteVMVal { return TValue<MiniMC::uint32_t>{val.getValue()}; },
                [](const MiniMC::Model::I64Integer& val) -> ConcreteVMVal { return TValue<MiniMC::uint64_t>{val.getValue()}; },
                [](const MiniMC::Model::Bool& val) -> ConcreteVMVal { return BoolValue{static_cast<bool>(val.getValue())}; },
                [](const MiniMC::Model::Pointer& val) -> ConcreteVMVal { return PointerValue{val.getValue()}; },
                [](const MiniMC::Model::AggregateConstant& val) -> ConcreteVMVal {
                  MiniMC::Util::Array res(val.begin(), val.end());
                  return AggregateValue({res});
                },
                [](const MiniMC::Model::Undef&) -> ConcreteVMVal { throw MiniMC::Support::Exception("Undef Values Not supported by Concrete CPA"); },
                [this](const MiniMC::Model::Register& val) -> ConcreteVMVal {
                  return lookupRegister (val);
                },
            },
            *v);
      }

      ConcreteVMVal ValueLookup::unboundValue(const MiniMC::Model::Type_ptr& t) const {
        switch (t->getTypeID()) {
          case MiniMC::Model::TypeID::Bool:
            return BoolValue(false);

          case MiniMC::Model::TypeID::Pointer:
            return PointerValue(MiniMC::Support::null_pointer());
          case MiniMC::Model::TypeID::I8:
            return TValue<MiniMC::uint8_t>(0);
          case MiniMC::Model::TypeID::I16:
            return TValue<MiniMC::uint16_t>(0);
          case MiniMC::Model::TypeID::I32:
            return TValue<MiniMC::uint32_t>(0);
          case MiniMC::Model::TypeID::I64:
            return TValue<MiniMC::uint64_t>(0);

          case MiniMC::Model::TypeID::Array:
          case MiniMC::Model::TypeID::Struct:
            return AggregateValue{MiniMC::Util::Array{t->getSize()}};
          default:
            break;
        }

        throw MiniMC::Support::Exception("Erro");
      }

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
