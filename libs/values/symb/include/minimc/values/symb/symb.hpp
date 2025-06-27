#ifndef _SYMB_VALUES__
#define _SYMB_VALUES__

#include "minimc/host/types.hpp"
#include "minimc/model/types.hpp"
#include "minimc/hash/hashing.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/vm/vmt.hpp"
#include "minimc/smt/smt.hpp"
#include "minimc/model/checkers/typechecker.hpp"
#include "smt/exceptions.hpp"
namespace MiniMC {
  namespace Values {
    namespace Symb {
      template<MiniMC::Model::TypeID TID> 
      class TValue : public MiniMC::Hash::RandomHash  {
      public:
	TValue () : value(nullptr) {}
	TValue (MiniMC::Model::Value_ptr&& v) : value(std::move(v)) {}
	auto& getExpr () const {assert(value); return value;}
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
	  static MiniMC::Model::Value_ptr makeIntegerValue(std::int64_t) ;
	  
	};
      
      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I8>::get()  {return MiniMC::Model::I8Type::get();}

      template<>
      MiniMC::Model::Value_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I8>::makeIntegerValue(std::int64_t v)  {return MiniMC::Model::I8Integer::make(v);}
      
      
      
      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I16>::get()  {return MiniMC::Model::I16Type::get();}

      template<>
      MiniMC::Model::Value_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I16>::makeIntegerValue(std::int64_t v)  {return MiniMC::Model::I16Integer::make(v);}
      
      
      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I32>::get()  {return MiniMC::Model::I32Type::get();}

      template<>
      MiniMC::Model::Value_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I32>::makeIntegerValue(std::int64_t v)  {return MiniMC::Model::I32Integer::make(v);}
      
      
      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I64>::get()  {return MiniMC::Model::I64Type::get();}

      template<>
      MiniMC::Model::Value_ptr IntegerTypeConverter<MiniMC::Model::TypeID::I64>::makeIntegerValue(std::int64_t v)  {return MiniMC::Model::I64Integer::make(v);}
      
      
      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::Pointer>::get()  {return MiniMC::Model::PointerType::get();}

      template<>
      MiniMC::Model::Type_ptr IntegerTypeConverter<MiniMC::Model::TypeID::Pointer32>::get()  {return MiniMC::Model::Pointer32Type::get();}

      class TypecheckedExpressionBuilder : public MiniMC::Model::ExpressionBuilder{
      public:
	MiniMC::Model::Value_ptr get () {
	  auto res = ExpressionBuilder::get();
	  if (typechecker.CheckType(*res)) {
	    return res;
	  }
	  else {
	    throw MiniMC::Support::Exception ("Not type correct expression");
	  }
	}
      private:
	MiniMC::Model::Checkers::TypeChecker typechecker {MiniMC::Support::Messager{}};
	
      };
      
      class Operations {
      public:
	template <typename T>
        T Not(const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  TypecheckedExpressionBuilder builder;
	  builder << l.getValue();
	  builder.Not();
	  return builder.get();
        }

	template<class T>
	Value::Pointer PtrAdd (const Value::Pointer& p, const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  TypecheckedExpressionBuilder builder;
	  builder << p.getValue()
		  << l.getValue();
	  builder.PtrAdd();
	  return builder.get();
        
	}

	template<class T>
	Value::Pointer PtrSub (const Value::Pointer& p, const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  TypecheckedExpressionBuilder builder;
	  builder << p.getValue()
		  << l.getValue();
	  builder.PtrSub();
	  return builder.get();
	}

	Value::Pointer32 PtrToPtr32 (const Value::Pointer& p) const  {
	    TypecheckedExpressionBuilder builder;
	    builder << p.getValue();
	    builder.PtrToPtr32 ();
	    return builder.get();
	}

	Value::Pointer Ptr32ToPtr (const Value::Pointer32& p) const {
	    TypecheckedExpressionBuilder builder;
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
	  TypecheckedExpressionBuilder builder;			\
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
	  TypecheckedExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.NN();							\
	  return Value::Bool{builder.get()};				\
	}
	
	OPS
	
#undef OPS
#undef X

	
	Value::Bool BoolNegate(const Value::Bool& l) const  { \
	  TypecheckedExpressionBuilder builder;			\
	  builder << l.getValue();					\
	  builder.LogNot();							\
	  return {builder.get()};				\
	}

	Value::Bool BoolAnd(const Value::Bool& l,const Value::Bool& r) const { \
	  TypecheckedExpressionBuilder builder;			\
	  builder << l.getValue() << r.getValue();			\
	  builder.LogAnd();							\
	  return {builder.get()};				\
	}

	template <class T>
	T ExtractBaseValue(const Value::Aggregate& value, const MiniMC::BV64 offset) const  {
	  TypecheckedExpressionBuilder builder;	\
	  builder << value.getValue() << MiniMC::Model::I64Integer::make(offset);
	  builder << IntegerTypeConverter<T::tid()>::get();
	  builder.ExtractValue();
	  return builder.get();
	  
	}
	
	Value::Aggregate ExtractAggregateValue(const Value::Aggregate& value, const MiniMC::BV64 offset, std::size_t size) const  {
	  TypecheckedExpressionBuilder builder;	\
	  builder << value.getValue() << MiniMC::Model::I64Integer::make(offset) << MiniMC::Model::AggregateType::get(size);
	  builder.ExtractValue();
	  return builder.get();
	  
	}

        template <class T>
        Value::Aggregate InsertBaseValue(const Value::Aggregate& aggrvalue, const MiniMC::BV64 offset, const T& insertee)  const requires (!MiniMC::VMT::MemoryC<Value,T>) {
	  TypecheckedExpressionBuilder builder;			\
	  builder << aggrvalue.getValue() << MiniMC::Model::I64Integer::make(offset) << insertee.getValue();
	  builder.InsertValue();
	  return builder.get ();
	}
	
	Value::Aggregate InsertAggregateValue(const Value::Aggregate& aggr, const MiniMC::BV64 offset, const Value::Aggregate& insertee) const {
	  TypecheckedExpressionBuilder builder;			\
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
	  TypecheckedExpressionBuilder builder;			\
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
	  TypecheckedExpressionBuilder builder;			\
	  builder << l.getValue() << IntegerTypeConverter<To>::get();	\
	  builder.NN ();						\
	  return TValue<To> {builder.get()};				\
	}
	
	OPS
#undef X
#undef OPS

	template<typename T>
	Value::Bool IntToBool (const T& l) const requires MiniMC::VMT::Integer<Value,T> {
	  TypecheckedExpressionBuilder builder;    
	  builder << l.getValue();
	  builder << MiniMC::Model::BoolType::get();
	  builder.IntToBool ();
	  return {builder.get()};
	}

	template<class To,class From>
	To BitCast (const From& f) const requires MiniMC::VMT::Aggregate<Value,To>  {
	  TypecheckedExpressionBuilder builder;    
	  builder << f.getValue();
	  builder << MiniMC::Model::AggregateType::get(f.getValue()->getType()->getSize());
	  builder.BitCast ();
	  return builder.get();
	}

	template<class To,class From>
	To BitCast (const From& f) const requires (!MiniMC::VMT::Aggregate<Value,To>)  {
	  TypecheckedExpressionBuilder builder;    
	  builder << f.getValue();
	  builder << IntegerTypeConverter<To::tid()>::get();
	  builder.BitCast ();
	  return builder.get();
	}
	
	Value::I8 create (const MiniMC::Model::I8Integer& v) const {
	  return Value::I8 {MiniMC::Model::I8Integer::make (v.getValue())};
	}


	Value::Pointer create (const MiniMC::Model::Pointer& v) const {
	  return Value::Pointer {MiniMC::Model::Pointer::make (v.getValue())};
	}

	Value::Pointer32 create (const MiniMC::Model::Pointer32& v) const {
	  return Value::Pointer32 {MiniMC::Model::Pointer32::make (v.getValue())};
	}
	
	Value::I16 create (const MiniMC::Model::I16Integer& v) const {
	  return Value::I16 {MiniMC::Model::I16Integer::make (v.getValue())};
	  
	}

	Value::I32 create (const MiniMC::Model::I32Integer& v) const {
	  return Value::I32 {MiniMC::Model::I32Integer::make (v.getValue())};
	  
	}

	Value::I64 create (const MiniMC::Model::I64Integer& v) const {
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

	template<std::size_t bytes,class T>
	std::generator<Value::I8> extract (const T& t) const {
	  TypecheckedExpressionBuilder builder;
	  for (std::size_t i = 0; i < bytes; i++) {
	    builder << t.getValue();
	    if (i > 0) {
	      builder << IntegerTypeConverter<T::tid()>::makeIntegerValue(i*8);
	      builder.AShr ();
	    }
	    builder << IntegerTypeConverter<MiniMC::Model::TypeID::I8>::get ();
	    builder.Trunc();
	    auto val = builder.get();
	    co_yield Value::I8{std::move(val)};
	  }
	}
	
	std::generator<Value::I8> bytes (const Value::I8& v) const{
	  co_yield v;
	}
	std::generator<Value::I8> bytes (const Value::I16& v) const {
	  co_yield std::ranges::elements_of (extract<2> (v));
	}

	std::generator<Value::I8> bytes (const Value::I32& v)const{
	  co_yield std::ranges::elements_of (extract<4> (v));
	}

	std::generator<Value::I8> bytes (const Value::I64& v)const{
	  co_yield std::ranges::elements_of (extract<8> (v));
	}
	std::generator<Value::I8> bytes (const Value::Pointer&)const{throw MiniMC::Support::Exception {"not implemented"};}
	std::generator<Value::I8> bytes (const Value::Pointer32&)const{throw MiniMC::Support::Exception {"not implemented"};}
	std::generator<Value::I8> bytes (const Value::Aggregate&)const{throw MiniMC::Support::Exception {"not implemented"};}

	
      };

      class MemoryOps {
      public:
	Value load(const Value::Memory&, const typename Value::Pointer&, const MiniMC::Model::Type& ty) const {return Operations{}.defaultValue(ty);}
	std::generator<Value::I8> loadBytes(const Value::Memory& m, const typename Value::Pointer& p , std::size_t bytes) const {
	  TypecheckedExpressionBuilder builder;
	  for (std::size_t i  = 0; i < bytes; ++i) {
	    builder << m.getValue();
	    builder << p.getValue();
	    if (i) {
	      builder << MiniMC::Model::I64Integer::make(i);
	      builder.PtrAdd ();
	    }
	    builder << IntegerTypeConverter<MiniMC::Model::TypeID::I8>::get();
	    builder.Load();
	    auto val = builder.get();
	    co_yield Value::I8{std::move(val)};
	  }
	}

	
	Value::Memory store(const Value::Memory& m, const Value::Pointer& p, const Value::I8& val) const  {
	  TypecheckedExpressionBuilder builder;    
	  builder << m.getValue() << p.getValue() << val.getValue();
	  builder.Store();
	  return builder.get();
	}
	
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
	ConstraintSolver () : context(MiniMC::Support::SMT::SMTSolverRepository::get().getBackend("CVC4").value().makeContext()),solver(context->getSolver()),translator(context) {
	}
	
	void push () {solver.push();}
	void pop () {solver.pop();}	
	void addConstraint (Value::Bool b) {
	  auto form = translator.Translate(*b.getExpr());
	  solver.assert_formula(form);
	  
	}
	MiniMC::VMT::Feasibility check () const {
	  switch (solver.check_sat()) {
	  case SMTLib::Result::Satis:
	    return MiniMC::VMT::Feasibility::Feasible;
	  case SMTLib::Result::NSatis:
	    return MiniMC::VMT::Feasibility::Infeasible;
	  default:
	    return MiniMC::VMT::Feasibility::Unknown;
	  }
	  return MiniMC::VMT::Feasibility::Feasible; 
	}
	
	MiniMC::Model::Constant_ptr eval (const Value& v) const {
	  auto translated = Value::visit ([this](const auto& vv) {return translator.Translate(*vv.getExpr());},v);
	  try {
	    auto val = solver.getModelValue(translated);
	    return Value::visit (MiniMC::Support::Overload {
	      [&val](const Value::I8& ) ->MiniMC::Model::Constant_ptr {
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::BV8 res;
		MiniMC::Support::SMT::extractByte (ires.begin(),res);
		
		return MiniMC::Model::I8Integer::make(res);
	      },
	      [&val](const Value::I16& ) ->MiniMC::Model::Constant_ptr {
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::BV16 res;
		MiniMC::Support::SMT::extract (ires.begin(),res);
		
		return MiniMC::Model::I16Integer::make(res);

	      },
	      [&val](const Value::I32& ) ->MiniMC::Model::Constant_ptr {
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::BV32 res{0};
		MiniMC::Support::SMT::extract (ires.begin(),res);
		
		return MiniMC::Model::I32Integer::make(res);

	      },
	      [&val](const Value::I64& ) ->MiniMC::Model::Constant_ptr{
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::BV64 res;
		MiniMC::Support::SMT::extract (ires.begin(),res);
		
		return MiniMC::Model::I64Integer::make(res);

	      },
	      [&val](const Value::Pointer& ) ->MiniMC::Model::Constant_ptr {
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::Model::pointer_t res = MiniMC::Model::pointer_t::makeNullPointer();
		decltype(res.segment) seg{0};
		decltype(res.offset) offset{0};
		decltype(res.base) base{0};

		
		auto iter = MiniMC::Support::SMT::extract (ires.begin(),offset);
		iter = MiniMC::Support::SMT::extract (iter,base);
		iter = MiniMC::Support::SMT::extract (iter,seg);
		res.segment = seg;
		res.offset = offset;
		res.base = base;
		return MiniMC::Model::Pointer::make (res);
		
	      },
	      [&val](const Value::Pointer32& ) ->MiniMC::Model::Constant_ptr {
		auto ires = std::get<SMTLib::bitvector> (val);
		MiniMC::Model::pointer32_t res = MiniMC::Model::pointer32_t::makeNullPointer();
		decltype(res.segment) seg{0};
		decltype(res.offset) offset{0};
		decltype(res.base) base{0};

		
		auto iter = MiniMC::Support::SMT::extract (ires.begin(),offset);
		iter = MiniMC::Support::SMT::extract (iter,base);
		iter = MiniMC::Support::SMT::extract (iter,seg);
		res.segment = seg;
		res.offset = offset;
		res.base = base;
		
		return MiniMC::Model::Pointer32::make (res);
		
	      },
	      [&val](const Value::Bool& ) ->MiniMC::Model::Constant_ptr {
		  auto bres = std::get<bool>(val);
		  return MiniMC::Model::Bool::make(bres);
	  
	      },
		[&val](const Value::Aggregate& a) ->MiniMC::Model::Constant_ptr {
		  MiniMC::Util::Array res{a.getExpr()->getType()->getSize()};
	  
		  auto aggrres = std::get<SMTLib::bitvector>(val);
		  auto iter = aggrres.begin();
		  for (size_t i = 0; i < a.getExpr()->getType()->getSize(); i++) {
		    MiniMC::BV8 buf;
		    iter = MiniMC::Support::SMT::extractByte (iter,buf);
		    res.get_direct_access ()[i] = buf;
		  }
		  return MiniMC::Model::AggregateConstant::make(std::move(res));
	  
	      },
		[](Value::Memory&) ->MiniMC::Model::Constant_ptr {return MiniMC::Model::I8Integer::make  (0);}
		
		},v
	    );
	    
	    
	  }catch(SMTLib::Exception& e) {
	    return MiniMC::Model::I8Integer::make(0);
	  }
	  
	}
      private:
	SMTLib::Context_ptr context;
	SMTLib::Solver& solver;
	MiniMC::Support::SMT::Translator translator;
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
