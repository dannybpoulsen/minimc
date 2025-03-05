#include "minimc/model/valuevisitor.hpp"
#include "minimc/host/casts.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/values/concrete/concrete.hpp"
#include "minimc/model/nondet_generator.hpp"

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
      Value Operations::defaultValue(const MiniMC::Model::Type& t) const {
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
	case MiniMC::Model::TypeID::Memory:
	  return Value::Memory{};
	default:
	  break;
        }
	
        throw MiniMC::Support::Exception("Erro");
      }
      

      std::generator<Value> Operations::create (const MiniMC::Model::Undef& und) const {
	MiniMC::Model::NonDetGenerator gen;
	for (auto t  : gen.generate(*und.getType ())) {
	  co_yield MiniMC::Model::visitValue<Value> (
						     MiniMC::Support::Overload {
						       [this]<typename T>(T& v)->Value requires (MiniMC::Model::is_root<T> && ! MiniMC::Model::is_register<T> && !MiniMC::Model::is_symbolic<T> ) {
							 return this->create(v);
						       },
							 MiniMC::Support::Error<Value> {}
					      },
						     *t
						     );
	  
				 
	}
	
      }
      

      MiniMC::Model::Constant_ptr ConstraintSolver::eval (const Value& v) const {
	return MiniMC::VMT::Concrete::Value::visit (MiniMC::Support::Overload {
	    [](MiniMC::VMT::Concrete::Value::I8& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::I16& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I16Integer::make  (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::I32& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I32Integer::make (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::I64& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::I64Integer::make  (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::Pointer& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Pointer::make (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::Pointer32& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Pointer32::make (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::Bool& val) ->MiniMC::Model::Constant_ptr{return MiniMC::Model::Bool::make (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::Aggregate& val) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::AggregateConstant::make  (val.getValue ());},
	      [](MiniMC::VMT::Concrete::Value::Memory&) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make (0);}
	      },
	  v
	  );
	
	
      }
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
