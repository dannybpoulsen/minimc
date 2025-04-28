#ifndef _SYMB_VALUES__
#define _SYMB_VALUES__

#include "minimc/model/types.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"

namespace MiniMC {
  namespace Values {
    namespace Symb {
      template<MiniMC::Model::TypeID TID> 
      class TValue : public MiniMC::Hash::RandomHash  {
      public:
	TValue (MiniMC::Model::Value_ptr&& v) : value(std::move(v)) {}
	auto& getEpr () const {return value;}
	std::ostream& output(std::ostream& os) const {
	  return value->output (os);
	}

	MiniMC::VMT::TriBool boolState () const {return MiniMC::VMT::TriBool::Unk;}

	auto getValue() const {return value;}
	static constexpr MiniMC::Model::TypeID tid ()  {return TID;}
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
      
	template<MiniMC::Model::TypeID To>
	struct IntegerTypeConverter {
	  MiniMC::Model::Type_ptr get() const {return MiniMC::Model::VoidType::get();}
	};

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I8>::get() const {return MiniMC::Model::I8Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I16>::get() const {return MiniMC::Model::I16Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I32>::get() const {return MiniMC::Model::I32Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I64>::get() const {return MiniMC::Model::I64Type::get();}
      
      
      class Operations {
	template <typename T>
        T Not(const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  MiniMC::Model::ExpressionBuilder builder;
	  builder << l.getValue();
	  builder.Not();
	  return builder.get();
        }

	template<class T>
	Value::Pointer PtrAdd (const Value::Pointer& p, const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  MiniMC::Model::ExpressionBuilder builder;
	  builder << p.getValue()
		  << l.getValue();
	  builder.PtrAdd();
	  return builder.get();
        
	}

	template<class T>
	Value::Pointer PtrSub (const Value::Pointer& p, const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  MiniMC::Model::ExpressionBuilder builder;
	  builder << p.getValue()
		  << l.getValue();
	  builder.PtrSub();
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
	X(NEq)						\


#define X(NN)                                   \
	template <typename T>						\
	Value::Bool NN(const T& l, const T& r) const  requires MiniMC::VMT::Integer<Value, T>{ \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.NN();							\
	  return Value::Bool{builder.get()};				\
	}
	
	OPS
	
#undef OPS
#undef X

#define OPS					\
	X(Not)					\
	X(LogNot)					\

#define X(NN)					\
	template<typename T>						\
	Value::Bool NN(const T& l) const  requires MiniMC::VMT::Integer<Value, T>{ \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue();					\
	  builder.NN();							\
	  return Value::Bool{builder.get()};				\
	}

	OPS
#undef OPS
#undef X
	
	Value::Aggregate ExtractAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, std::size_t size) const  {
	  MiniMC::Model::ExpressionBuilder builder;	\
	  builder << value.getValue() << MiniMC::Model::I64Integer::make(offset) << MiniMC::Model::AggregateType::get(size);
	  builder.ExtractValue();
	  return builder.get();
	  
	}

        template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate& aggrvalue, const MiniMC::BV64 offset, const T& insertee)  const requires (!MiniMC::VMT::MemoryC<Value,T>) {
	  MiniMC::Model::ExpressionBuilder builder;	\
	  builder << aggrvalue.getValue() << MiniMC::Model::I64Integer::make(offset) << insertee.getValue();
	  builder.InsertValue();
	  return builder.get ();
	}
	
	Value::Aggregate InsertAggregateValue(const Value::Aggregate& aggr, const MiniMC::BV64 offset, const Value::Aggregate& insertee) const {
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << aggr.getValue() << MiniMC::Model::I64Integer::make(offset) << insertee.getValue();
	  builder.InsertValue();
	  return builder.get ();
	  
	}

	
#define OPS						\
	X(ZExt)						\
	X(SExt)						\
	X(Trunc)					\
	
#define X(NN)								\
	template<MiniMC::Model::TypeID To,class T>			\
	TValue<To> NN(const T& l) const  requires MiniMC::VMT::Integer<Value, T>{ \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << IntegerTypeConverter<To>::get();	\
	  builder.NN ();						\
	  return TValue<To> {builder.get()};				\
	}
	
	OPS
#undef OPS
#undef X

	

	template<class T>						
	T PtrToInt(const Value::Pointer& l) const  requires MiniMC::VMT::Integer<Value, T>{ 
	  MiniMC::Model::ExpressionBuilder builder;			
	  builder << l.getValue() << IntegerTypeConverter<T::tid()>::get();	
	  builder.PtrToInt ();						
	  return T {builder.get()};				
	}

	template <class T>
        T Ptr32ToInt(const Value::Pointer32& l) const {
	  MiniMC::Model::ExpressionBuilder builder;			
	  builder << l.getValue() << MiniMC::Model::I32Type::get();
	  builder.BitCast ();
	  builder << IntegerTypeConverter<T::tid()>::get();
	  if constexpr (T::tid() == MiniMC::Model::TypeID::I64) {
	    builder.ZExt();
	    return builder.get();
	  }
	  else if constexpr (T::Tid() == MiniMC::Model::TypeID::I32)
	    return builder.get();
	  else {
	    builder.Trunc();
	    return builder.get();
	  }
	  
	}


      };
      
    }
  }
}

#endif 
