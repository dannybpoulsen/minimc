#ifndef _SYMB_VALUES__
#define _SYMB_VALUES__

#include "minimc/model/types.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"

namespace MiniMC {
  namespace Values {
    namespace Symb {
      template<MiniMC::Model::TypeID> 
      class TValue : public MiniMC::Hash::RandomHash  {
      public:
	TValue (MiniMC::Model::Value_ptr&& v) : value(std::move(v)) {}
	auto& getEpr () const {return value;}
	std::ostream& output(std::ostream& os) const {
	  return value->output (os);
	}

	MiniMC::VMT::TriBool boolState () const {return MiniMC::VMT::TriBool::Unk;}
	
	
      private:
	MiniMC::Model::Value_ptr value;
      };

      template<MiniMC::Model::TypeID id>
      inline std::ostream& operator<<(std::ostream& os, const TValue<id>& val) { return val.output(os); }

      using Value = MiniMC::VMT::GenericVal<TValue<MiniMC::Model::TypeID::I8>,
					    TValue<MiniMC::Model::TypeID::I16>,
					    TValue<MiniMC::Model::TypeID::I32>,
					    TValue<MiniMC::Model::TypeID::I64>,
					    TValue<MiniMC::Model::TypeID::Pointer>,
					    TValue<MiniMC::Model::TypeID::Pointer32>,
					    TValue<MiniMC::Model::TypeID::Bool>,
					    TValue<MiniMC::Model::TypeID::Aggregate>,
					    TValue<MiniMC::Model::TypeID::Memory>
					    >;
      

      class Operations {
	template <typename T>
        T Not(const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  MiniMC::Model::ExpressionBuilder builder;
	  builder << l.getValue();
	  builder.Not();
	  return builder.get();
        }


#define OPS					\
	X(Add)					\
	X(Sub)					\
	X(Mul)					\
	X(UDiv)					\
	X(SDiv)					\
	X(LShl)					\
	X(LShr)					\
	X(AShr)					\
	X(And)					\
	X(Or)					\
	X(Xor)					\
	

#define X(NN)								\
	template<typename T>						\
	T NN(const T& l, const T& t) const requires MiniMC::VMT::Integer<Value,T> { \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.NN();							\
	  return builder.get();						\
									\
	}								\
	OPS
#undef X
#undef OPS	
	
#define OPS						\
	X(SGt)						\
	X(SGe)						\
	X(UGt)						\
	X(UGe)						\
	X(SLt)						\
	X(SLe)						\
	X(ULt)						\
	X(ULe)						\
	X(Eq)						\
	X(NEq)
	
#define X(NN)                                   \
	template <typename T>						\
	Value::Bool NN(const T& l, const T& r) const  requires MiniMC::VMT::Integer<Value, T>{ \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.NN();							\
	  return Value::Bool{builder.get()};				\
	}
	
	OPS
	
	
      };
      
    }
  }
}

#endif 
