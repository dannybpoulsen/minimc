#include "vm/value.hpp"
#include "model/variables.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include <type_traits>

#include "memory.hpp"
#include "model/variables.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include "value.hpp"
#include "vm/value.hpp"
#include <type_traits>

#include "memory.hpp"
#include "value.hpp"
#include "vm/vm.hpp"

namespace MiniMC {
  namespace VM {
    namespace Concrete {

      struct TPathControl : public PathControl, std::enable_shared_from_this<TPathControl> {
        virtual void addAssumption(const Value_ptr) {}
        virtual void addAssert(const Value_ptr) {}
        virtual PathControl_ptr copy() { return this->shared_from_this(); }
      };

      struct TValueLookup : public ValueLookup {
      public:
        TValueLookup(std::size_t s) : values(s) {}
        virtual Value_ptr lookupValue(const MiniMC::Model::Value_ptr& v) const override {
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

        virtual void saveValue(const MiniMC::Model::Variable_ptr& v, Value_ptr&& toset) override {
          values[v] = std::move(toset);
        }

        virtual std::shared_ptr<ValueLookup> copy() const override { return std::make_shared<TValueLookup>(*this); }
	
        MiniMC::Hash::hash_t hash() const override {
	  return values.hash(0);
        }

        virtual Value_ptr unboundValue(const MiniMC::Model::Type_ptr& t) const override {
          switch (t->getTypeID()) {
            case MiniMC::Model::TypeID::Integer:
              switch (t->getSize()) {
                case 1:
                  return std::make_shared<TValue<MiniMC::uint8_t>>(0);
                  break;
                case 2:
                  return std::make_shared<TValue<MiniMC::uint16_t>>(0);
                  break;
                case 4:
                  return std::make_shared<TValue<MiniMC::uint32_t>>(0);
                  break;
                case 8:
                  return std::make_shared<TValue<MiniMC::uint64_t>>(0)  ;
                  break;
              }
	      break;
          case MiniMC::Model::TypeID::Bool:
	  return std::make_shared<BoolValue>(false);
	
	  case MiniMC::Model::TypeID::Pointer:
	    return std::make_shared<PointerValue>(MiniMC::Support::null_pointer());
	    
	  case MiniMC::Model::TypeID::Struct:
	  case MiniMC::Model::TypeID::Array:
	      
            return std::make_shared<AggregateValue>(MiniMC::Util::Array{t->getSize()});
	  default:
	    break;
	  }
	  throw MiniMC::Support::Exception ("Not support type");
	    
	}
	  
        private:
          MiniMC::Model::VariableMap<MiniMC::VM::Value_ptr> values;
        };

        Value_ptr BoolValue::BoolSExt(const MiniMC::Model::Type_ptr& t) {

          auto performSExt = [&]<typename To>(auto& val) {
            if (val)
              return std::make_shared<TValue<To>>(std::numeric_limits<To>::max());
            else
              return std::make_shared<TValue<To>>(0);
          };
          switch (t->getSize()) {
            case 1:
              return performSExt.template operator()<MiniMC::uint8_t>(value);
            case 2:
              return performSExt.template operator()<MiniMC::uint16_t>(value);
            case 4:
              return performSExt.template operator()<MiniMC::uint32_t>(value);
            case 8:
              return performSExt.template operator()<MiniMC::uint64_t>(value);
          }
          throw MiniMC::Support::Exception("Improper extension");
        }
        Value_ptr BoolValue::BoolZExt(const MiniMC::Model::Type_ptr& t) {
          auto performZExt = [&]<typename To>(auto& val) {
            if (val)
              return std::make_shared<TValue<To>>(1);
            else
              return std::make_shared<TValue<To>>(0);
          };
          switch (t->getSize()) {
            case 1:
              return performZExt.template operator()<MiniMC::uint8_t>(value);
            case 2:
              return performZExt.template operator()<MiniMC::uint16_t>(value);
            case 4:
              return performZExt.template operator()<MiniMC::uint32_t>(value);
            case 8:
              return performZExt.template operator()<MiniMC::uint64_t>(value);
          }
          throw MiniMC::Support::Exception("Improper extension");
        }

        MiniMC::VM::ValueLookup_ptr makeLookup(std::size_t s) {
          return std::make_shared<TValueLookup>(s);
        }

        MiniMC::VM::PathControl_ptr makePathControl() {
          return std::make_shared<TPathControl>();
        }

      } // namespace Concrete
    }   // namespace VM
  }     // namespace MiniMC
