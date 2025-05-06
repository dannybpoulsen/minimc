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
	TValue () : value(nullptr) {}
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
	  static MiniMC::Model::Type_ptr get()  {return MiniMC::Model::VoidType::get();}
	};

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I8>::get()  {return MiniMC::Model::I8Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I16>::get()  {return MiniMC::Model::I16Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I32>::get()  {return MiniMC::Model::I32Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I64>::get()  {return MiniMC::Model::I64Type::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::Pointer>::get()  {return MiniMC::Model::PointerType::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::Pointer32>::get()  {return MiniMC::Model::Pointer32Type::get();}
      
      
      class Operations {
      public:
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

	Value::Pointer32 PtrToPtr32 (const Value::Pointer& p) const  {
	    MiniMC::Model::ExpressionBuilder builder;
	    builder << p.getValue();
	    builder.PtrToPtr32 ();
	    return builder.get();
	}

	Value::Pointer Ptr32ToPtr (const Value::Pointer32& p) const {
	    MiniMC::Model::ExpressionBuilder builder;
	    builder << p.getValue();
	    builder.Ptr32ToPtr ();
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
	T NN(const T& l, const T& r) const requires MiniMC::VMT::Integer<Value,T> { \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() <<  r.getValue();			\
	  builder.NN();							\
	  return builder.get();						\
									\
	}								
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

	
	Value::Bool BoolNegate(const Value::Bool& l) const  { \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue();					\
	  builder.LogNot();							\
	  return {builder.get()};				\
	}

	Value::Bool BoolAnd(const Value::Bool& l,const Value::Bool& r) const { \
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.LogAnd();							\
	  return {builder.get()};				\
	}

	template <class T>
	T ExtractBaseValue(const Value::Aggregate& value, const MiniMC::BV64 offset) const  {
	  MiniMC::Model::ExpressionBuilder builder;	\
	  builder << value.getValue() << MiniMC::Model::I64Integer::make(offset);
	  builder << IntegerTypeConverter<T::tid()>::get();
	  builder.ExtractValue();
	  return builder.get();
	  
	}
	
	Value::Aggregate ExtractAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, std::size_t size) const  {
	  MiniMC::Model::ExpressionBuilder builder;	\
	  builder << value.getValue() << MiniMC::Model::I64Integer::make(offset) << MiniMC::Model::AggregateType::get(size);
	  builder.ExtractValue();
	  return builder.get();
	  
	}

        template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate& aggrvalue, const MiniMC::BV64 offset, const T& insertee)  const requires (!MiniMC::VMT::MemoryC<Value,T>) {
	  MiniMC::Model::ExpressionBuilder builder;			\
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

#define OPS								\
	X(ZExt)								\
	X(SExt)								\
	
#define X(NN)								\
	template<MiniMC::Model::TypeID To>				\
	TValue<To> NN(const Value::Bool& l) const {				\
	  MiniMC::Model::ExpressionBuilder builder;			\
	  builder << l.getValue() << IntegerTypeConverter<To>::get();	\
	  builder.NN ();						\
	  return TValue<To> {builder.get()};				\
	}
	
	OPS
#undef X
#undef OPS

	template<typename T>
	Value::Bool IntToBool (const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  MiniMC::Model::ExpressionBuilder builder;    
	  builder << l.getValue();
	  builder << MiniMC::Model::BoolType::get();
	  builder.IntToBool ();
	  return {builder.get()};
	}

	template<class To,class From>
	To BitCast (const From& f) const requires MiniMC::VMT::Aggregate<Value,To>  {
	  MiniMC::Model::ExpressionBuilder builder;    
	  builder << f.getValue();
	  builder << MiniMC::Model::AggregateType::get(f.getValue()->getType()->getSize());
	  builder.BitCast ();
	  return builder.get();
	}

	template<class To,class From>
	To BitCast (const From& f) const requires (!MiniMC::VMT::Aggregate<Value,To>)  {
	  MiniMC::Model::ExpressionBuilder builder;    
	  builder << f.getValue();
	  builder << IntegerTypeConverter<To::tid()>::get();
	  builder.BitCast ();
	  return builder.get();
	}
	
	Value create (const MiniMC::Model::I8Integer& v) const {
	  return Value::I8 {MiniMC::Model::I8Integer::make (v.getValue())};
	}


	Value create (const MiniMC::Model::Pointer& v) const {
	  return Value::Pointer {MiniMC::Model::Pointer::make (v.getValue())};
	}

	Value create (const MiniMC::Model::Pointer32& v) const {
	  return Value::Pointer32 {MiniMC::Model::Pointer32::make (v.getValue())};
	}
	
	Value create (const MiniMC::Model::I16Integer& v) const {
	  return Value::I16 {MiniMC::Model::I16Integer::make (v.getValue())};
	  
	}

	Value create (const MiniMC::Model::I32Integer& v) const {
	  return Value::I32 {MiniMC::Model::I32Integer::make (v.getValue())};
	  
	}

	Value create (const MiniMC::Model::I64Integer& v) const {
	  return Value::I64 {MiniMC::Model::I64Integer::make (v.getValue())};
	  
	}

	Value::Bool create (const MiniMC::Model::Bool& v) const {
	  return Value::Bool {MiniMC::Model::Bool::make (v.getValue())};
	  
	}

	Value create (const MiniMC::Model::AggregateConstant& v) const {
	  auto data = v.getData();
	  return Value::Aggregate {MiniMC::Model::AggregateConstant::make (std::move(data))};
	  
	}


	Value defaultValue (const MiniMC::Model::Type& ty ) const {
	  switch (ty.getTypeID()) {
	  case MiniMC::Model::TypeID::I8:
	    return Value::I8{MiniMC::Model::Undef::make(MiniMC::Model::I8Type::get())};
	  case MiniMC::Model::TypeID::I16:
	    return Value::I16{MiniMC::Model::Undef::make(MiniMC::Model::I16Type::get())};
	  case MiniMC::Model::TypeID::I32:
	    return Value::I32{MiniMC::Model::Undef::make(MiniMC::Model::I32Type::get())};
	  case MiniMC::Model::TypeID::I64:
	    return Value::I64{MiniMC::Model::Undef::make(MiniMC::Model::I64Type::get())};
	  case MiniMC::Model::TypeID::Bool:
	    return Value::Bool{MiniMC::Model::Undef::make(MiniMC::Model::BoolType::get())};
	  case MiniMC::Model::TypeID::Aggregate:
	    return Value::Aggregate{MiniMC::Model::Undef::make(MiniMC::Model::AggregateType::get(ty.getSize()))};
	  case MiniMC::Model::TypeID::Memory:
	    return Value::Memory{MiniMC::Model::Undef::make(MiniMC::Model::MemoryType::get())};
	    
	  case MiniMC::Model::TypeID::Pointer:
	    return Value::Pointer{MiniMC::Model::Undef::make(MiniMC::Model::PointerType::get())};
	  case MiniMC::Model::TypeID::Pointer32:
	    return Value::Pointer32{MiniMC::Model::Undef::make(MiniMC::Model::Pointer32Type::get())};
	  
	  default:
	    throw MiniMC::Support::Exception ("should never get here");
	  }
	}
	
	std::generator<Value> create (const MiniMC::Model::Undef& undef ) const {
	  co_yield defaultValue (*undef.getType());
	}

	
	
      };

      class MemoryOps {
      public:
	Value load(const Value::Memory&, const typename Value::Pointer&, const MiniMC::Model::Type& ty) const {return Operations{}.defaultValue(ty);}
	Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::I8&) const  { return m;}
	Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::I16&)const  {return m;}
        Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::I32&) const {return m;}
        Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::I64&) const {return m;}
	Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::Aggregate&) const {return m;}
	Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::Pointer&) const {return m;}
	Value::Memory store(const Value::Memory& m, const Value::Pointer&, const Value::Pointer32&) const {return m;}
	
	// PArameter is size to allocate
	Value::Memory allocate(const Value::Memory& m, const Value::Pointer&,const Value::I64&) {return m;}	
	Value::Pointer find_space(const Value::Memory&, const Value::I64&) {return Value::Pointer{MiniMC::Model::Undef::make (MiniMC::Model::PointerType::get())};}
	
        Value::Memory free(const Value::Memory& m, const Value::Pointer&){return m;}  
      private:
      };

      class ConstraintSolver {
      public:
	void push () {}
	void pop () {}	
	void addConstraint (Value::Bool) {}
	MiniMC::VMT::Feasibility check () const {
	  return MiniMC::VMT::Feasibility::Feasible; 
	}
	
	MiniMC::Model::Constant_ptr eval (const Value& ) const {return MiniMC::Model::I8Integer::make(0);}
      };

      class ValueDefinition  {
      public:
	using Val = Value;
	Operations ops () const {return Operations{};}
	MemoryOps memops () const {return MemoryOps{};}
	ConstraintSolver solver () const  {return ConstraintSolver{};}
      };
      
    }
  }
}

#endif 
