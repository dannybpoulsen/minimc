#include "minimc/model/valuevisitor.hpp"
#include "minimc/host/casts.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/support/feedback.hpp"
#include "concvm/concrete.hpp"


namespace MiniMC {
  namespace VMT {
    namespace Concrete {
      /*Value ValueLookupBase::lookupValue(const MiniMC::Model::Value& v) const {
	return MiniMC::Model::visitValue(
					 
	       MiniMC::Model::Overload{
		 [this](const MiniMC::Model::Register& val) -> Value {
		   return lookupRegisterValue (val);
		 },
		 [this](const auto& v) -> Value {
		   return creator.create (v);
		 }
            },
            v);
      }
      */
      Value ValueCreator::defaultValue(const MiniMC::Model::Type& t) const {
	switch (t.getTypeID()) {
	case MiniMC::Model::TypeID::Bool:
	  return BoolValue(0);
	case MiniMC::Model::TypeID::Pointer32:
	  return Value::Pointer32(Value::Pointer32::underlying_type {});
	  
	case MiniMC::Model::TypeID::Pointer:
	  return Value::Pointer(Value::Pointer::underlying_type {});
	case MiniMC::Model::TypeID::I8:
	  return Value::I8(0);
	case MiniMC::Model::TypeID::I16:
	  return Value::I16(0);
	case MiniMC::Model::TypeID::I32:
	  return Value::I32(0);
	case MiniMC::Model::TypeID::I64:
	  return Value::I64(0);

	case MiniMC::Model::TypeID::Aggregate:
	  return Value::Aggregate{MiniMC::Util::Array{t.getSize()}};
	default:
	  break;
        }
	
        throw MiniMC::Support::Exception("Erro");
      }
      
      
      Value ValueCreator::unboundValue(const MiniMC::Model::Type& t) const {
	MiniMC::Support::Messager{} << MiniMC::Support::TWarning {"Getting nondeterministic values for concrete values - using default value"};
	return defaultValue (t);
      }

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
