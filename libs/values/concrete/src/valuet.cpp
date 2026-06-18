#include "minimc/model/valuevisitor.hpp"
#include "minimc/host/casts.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/support/feedback.hpp"
#include "minimc/values/concrete/concrete.hpp"

namespace MiniMC {
  namespace VMT {
    namespace Concrete {

      
      
      template<class ND>
      std::generator<Value> Operations<ND>::create (const MiniMC::Model::Undef& und) const {
	for (auto t  : generator.generate(*und.getType ())) {
	  co_yield MiniMC::Model::visitValue (
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

      template
      class  Operations<MiniMC::Model::NonDetGenerator>;
      
      template
      class Operations<MiniMC::Model::StochasticGenerator>;
      
      
    } // namespace Concrete
  }   // namespace VMT
} // namespace MiniMC
