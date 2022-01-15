#include "vm/value.hpp"
#include "model/variables.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include <type_traits>

#include "value.hpp"
#include "memory.hpp"

namespace MiniMC {
  namespace VM {
    namespace Concrete {


      struct TValueLookup : public ValueLookup {
      public:
        TValueLookup(MiniMC::Model::VariableMap<MiniMC::VM::Value_ptr>& v) : values(v) {}
        virtual Value_ptr lookupValue(const MiniMC::Model::Value_ptr& v) {
          if (v->isConstant()) {
            auto constant = std::static_pointer_cast<MiniMC::Model::Constant>(v);
            if (constant->isInteger()) {
              switch (constant->getSize()) {
                case 1:
                  return std::make_shared<TValue<MiniMC::uint8_t>>(std::static_pointer_cast<MiniMC::Model::I8Integer>(constant)->getValue());
                  break;
                case 2:
                  return std::make_shared<TValue<MiniMC::uint16_t>>(std::static_pointer_cast<MiniMC::Model::I16Integer>(constant)->getValue());
                  break;
                case 4:
                  return std::make_shared<TValue<MiniMC::uint32_t>>(std::static_pointer_cast<MiniMC::Model::I32Integer>(constant)->getValue());
                  break;
                case 8:
                  return std::make_shared<TValue<MiniMC::uint64_t>>(std::static_pointer_cast<MiniMC::Model::I64Integer>(constant)->getValue());
                  break;
              }
            }

            else if (constant->isBool()) {
              return std::make_shared<BoolValue>(std::static_pointer_cast<MiniMC::Model::Bool>(constant)->getValue());
            }

            else if (constant->isPointer()) {
              return std::make_shared<PointerValue>(std::static_pointer_cast<MiniMC::Model::Pointer>(constant)->getValue());
            }

            else if (constant->isBinaryBlobConstant()) {
              auto blob = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant>(constant);
              return std::make_shared<AggregateValue>(MiniMC::Util::Array{blob->getSize(), blob->getData()});
            }

            throw MiniMC::Support::Exception("Not Implemented");
          } else {
            return values[std::static_pointer_cast<MiniMC::Model::Register>(v)];
          }
        }

        virtual void
        saveValue(const MiniMC::Model::Variable_ptr& v, Value_ptr&& toset) {
          values[v] = std::move(toset);
        }

      private:
        MiniMC::Model::VariableMap<MiniMC::VM::Value_ptr>& values;
      };

    } // namespace Concrete
  }   // namespace VM
} // namespace MiniMC
