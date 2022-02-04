#include "support/casts.hpp"
#include "support/operataions.hpp"
#include "support/pointer.hpp"
#include "vm/concrete/concrete.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      ConcreteVMVal ValueLookup::lookupValue(const MiniMC::Model::Value_ptr& v) const {
	
	if (v->isConstant()) {
          auto constant = std::static_pointer_cast<MiniMC::Model::Constant>(v);
          if (constant->isInteger()) {
            switch (constant->getSize()) {
              case 1:
                return TValue<MiniMC::uint8_t>(std::static_pointer_cast<MiniMC::Model::I8Integer>(constant)->getValue());
              case 2:
                return TValue<MiniMC::uint16_t>(std::static_pointer_cast<MiniMC::Model::I16Integer>(constant)->getValue());
              case 4:
                return TValue<MiniMC::uint32_t>(std::static_pointer_cast<MiniMC::Model::I32Integer>(constant)->getValue());
              case 8:
                return TValue<MiniMC::uint64_t>(std::static_pointer_cast<MiniMC::Model::I64Integer>(constant)->getValue());
            }
          }

          else if (constant->isBool()) {
            return BoolValue(std::static_pointer_cast<MiniMC::Model::Bool>(constant)->getValue());
          }

          else if (constant->isPointer()) {
            return PointerValue(std::static_pointer_cast<MiniMC::Model::Pointer>(constant)->getValue());
          }

	  else if (constant->isBinaryBlobConstant ()) {
	    auto bconstant = std::static_pointer_cast<MiniMC::Model::BinaryBlobConstant> (constant);
	    MiniMC::Util::Array val (bconstant->getSize (), bconstant->getData ());
	    return AggregateValue({val});
          
	  }
	  
          throw MiniMC::Support::Exception("Not Implemented");
        } else {
	  std::cerr << values[std::static_pointer_cast<MiniMC::Model::Register>(v)] << std::endl;
          return values[std::static_pointer_cast<MiniMC::Model::Register>(v)];
        }
      }

      ConcreteVMVal ValueLookup::unboundValue(const MiniMC::Model::Type_ptr& t) const {
        switch (t->getTypeID()) {
          case MiniMC::Model::TypeID::Bool:
            return BoolValue(false);

          case MiniMC::Model::TypeID::Pointer:
            return PointerValue(MiniMC::Support::null_pointer());
          case MiniMC::Model::TypeID::Integer: {
            switch (t->getSize()) {
              case 1:
                return TValue<MiniMC::uint8_t>(0);
              case 2:
                return TValue<MiniMC::uint16_t>(0);
              case 4:
                return TValue<MiniMC::uint32_t>(0);
              case 8:
                return TValue<MiniMC::uint64_t>(0);
            }
            std::cerr << "unknown size " << t->getSize() << std::endl;
          } break;

	case MiniMC::Model::TypeID::Array:
	case MiniMC::Model::TypeID::Struct:
	  return AggregateValue {MiniMC::Util::Array{t->getSize ()}};
          default:
            break;
        }

        throw MiniMC::Support::Exception("Erro");
      }

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
