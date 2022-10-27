#include "model/valuevisitor.hpp"
#include "host/casts.hpp"
#include "host/operataions.hpp"
#include "support/feedback.hpp"
#include "vm/concrete/concrete.hpp"


namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      ValueLookup::Value ValueLookup::lookupValue(const MiniMC::Model::Value_ptr& v) const {
	return MiniMC::Model::visitValue(
					 
		MiniMC::Model::Overload{
		 [](const MiniMC::Model::I8Integer& val) -> Value { return Value::I8{val.getValue()}; },
		 [](const MiniMC::Model::I16Integer& val) -> Value { return Value::I16{val.getValue()}; },
		 [](const MiniMC::Model::I32Integer& val) -> Value { return Value::I32{val.getValue()}; },
		 [](const MiniMC::Model::I64Integer& val) -> Value { return Value::I64{val.getValue()}; },
		 [](const MiniMC::Model::Bool& val) -> Value { return Value::Bool{static_cast<bool>(val.getValue())}; },
		 [](const MiniMC::Model::Pointer& val) -> Value { return Value::Pointer{val.getValue()}; },
		 [](const MiniMC::Model::Pointer32& val) -> Value { return Value::Pointer32{val.getValue()}; },
		 [](const MiniMC::Model::AggregateConstant& val) -> Value {
                  MiniMC::Util::Array res(val.begin(), val.end());
                  return AggregateValue({res});
                },
                [](const MiniMC::Model::Undef&) ->  Value { throw MiniMC::Support::Exception("Undef Values Not supported by Concrete CPA"); },
                [this](const MiniMC::Model::Register& val) -> Value {
                  return lookupRegister (val);
                },
            },
            *v);
      }

      ValueLookup::Value ValueLookup::defaultValue(const MiniMC::Model::Type_ptr& t) const {
	switch (t->getTypeID()) {
	case MiniMC::Model::TypeID::Bool:
            return BoolValue(false);
	case MiniMC::Model::TypeID::Pointer32:
	  return Value::Pointer32{Value::Pointer32::underlying_type {}};
	  
	case MiniMC::Model::TypeID::Pointer:
	  return Value::Pointer{PointerValue::underlying_type {}};
	case MiniMC::Model::TypeID::I8:
	  return Value::I8(0);
	case MiniMC::Model::TypeID::I16:
            return Value::I16(0);
	case MiniMC::Model::TypeID::I32:
	  return Value::I32(0);
	case MiniMC::Model::TypeID::I64:
	  return Value::I64(0);

	case MiniMC::Model::TypeID::Array:
	case MiniMC::Model::TypeID::Struct:
	  return Value::Aggregate{MiniMC::Util::Array{t->getSize()}};
	default:
	  break;
        }
	
        throw MiniMC::Support::Exception("Erro");
      }
      
      
      ValueLookup::Value ValueLookup::unboundValue(const MiniMC::Model::Type_ptr& t) const {
	MiniMC::Support::Messager{}.message<MiniMC::Support::Severity::Warning> ("Getting Nondeterministic values for concrete values....using default value");
	return defaultValue (t);
      }

    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
