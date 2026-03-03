#ifndef _VM_VMT__
#define _VM_VMT__

#include "minimc/hash/hashing.hpp"
#include "minimc/model/types.hpp"
#include "minimc/model/variables.hpp"
#include "minimc/vm/value.hpp"
#include "minimc/model/cfg.hpp"
#include "minimc/model/instructions.hpp"
#include "minimc/model/heaplayout.hpp"
#include "minimc/model/valuevisitor.hpp"
#include "minimc/support/overload.hpp"
#include <type_traits>
#include <iostream>
#include <utility>
#include <generator>

namespace MiniMC {
  namespace Model {
    class Program;
  }
  namespace VMT {
    
    template<MiniMC::Model::VMInstructionCode c>
    class NotImplemented : public MiniMC::Support::Exception {
    public:
      NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not supported."}.format (c)) {}
    };


    template<class c>
    class NotImplementedExpr : public MiniMC::Support::Exception {
    public:
      NotImplementedExpr () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Expression '%1%' not supported."}.format (typeid(c).name())) {}
    };


    enum class FlagType {
      AssertViolated = 0,
    };
  
    
    template<class Eval,class T>
    concept RegisterStore = requires (MiniMC::Model::Symbol s, const MiniMC::Model::Register& reg, const Eval& ceval, Eval& eval,  T&& t, const T::Pointer p,const T::Memory& mem,const MiniMC::Model::Type& ty, const T& value,std::size_t bytes, const T::I64&si64) {
      {ceval.lookupRegister (reg)} -> std::convertible_to<T>;
      {ceval.lookupSymbol (s)} -> std::convertible_to<T>;
      
      {eval.saveValue (reg,std::move(t))};
      {ceval.load(p,mem,ty)}->std::convertible_to<T>;
      {ceval.loadBytes(p,mem,bytes)}->std::convertible_to<std::generator<typename T::I8>>;
      
      {ceval.store(mem,p,value)}->std::convertible_to<typename T::Memory>;
      {ceval.find_space(mem,si64)}->std::convertible_to<T>;
      {ceval.check_free(mem,p,si64)}->std::convertible_to<T>;
      {ceval.allocate(mem,p,si64)}->std::convertible_to<T>;
      {ceval.free(mem,p)}->std::convertible_to<T>;
      
    } ;
    
    
    template<class StackC>
    concept StackControl = requires (
				     StackC& p,
				     MiniMC::Model::Location_ptr loc,				     
				     MiniMC::Model::Register_ptr value
				     )
    {
      {p.push (loc,value)};
      {p.pop ()}->std::convertible_to<MiniMC::Model::Value_ptr>;
    };

    
    
    


    template<class State>
    concept StackControllable = requires (State& state, MiniMC::Model::proc_t id) {
      {state.getStackControl (id)} ->StackControl;
    };

    


    template<class State,typename T>
    concept HasMemory = requires (State& state) {
      {state.getMemory ()} ->MemoryOperations<T>;
      
    };

    template<class State,typename Memory>
    concept HasMemoryNew = requires (State& state,Memory&& m) {
      {state.getMemory ()} ->std::convertible_to<Memory&>;
      {state.setMemory (m)};
      
    };

    
    template<class State,typename T>
    concept ValueLookupable = requires (State& state, MiniMC::Model::proc_t id) {
      {state.makeEvaluationContext (id)} ->RegisterStore<T>;
    };

    
    template<class State,class V>
    concept VMState =  StackControllable<State> &&
      ValueLookupable<State,V> && requires (State& s,typename V::Bool&& v,FlagType t) {
      {s.getPathform()}->std::convertible_to<typename V::Bool>;
      {s.setPathform(std::move(v))};
      {s.constraint_solver ()} -> ConstraintSolver<V>;
      {s.setFlag(t)};
    };
    
    template<class T,class R>
    concept Integer = std::is_same_v<R,typename T::I8> || std::is_same_v<R,typename T::I16> || std::is_same_v<R,typename T::I32> || std::is_same_v<R,typename T::I64>;

    template<class T,class R>
    concept Boolean = std::is_same_v<R,typename T::Bool>;    

    template<class T,class R>
    concept MemoryC = std::is_same_v<R,typename T::Memory>;    
    
    
    template<class T,class R>
    concept Pointer = std::is_same_v<R,typename T::Pointer> || std::is_same_v<R,typename T::Pointer32>;

    template<class T,class R>
    concept Aggregate = std::is_same_v<R,typename T::Aggregate> ;
    
    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    class MultiEvaluator {
    public:
      MultiEvaluator (Operations ops, const RegStore regstore) : ops(ops),regstore(std::move(regstore)) {}
      
      
      Value Eval (const MiniMC::Model::Value& v)  const {
	for (Value t :  MEval (v)) {
	  return t;
	}
	throw MiniMC::Support::Exception ("Generation Failed");
      }
      
      std::generator<Value> MEval (const MiniMC::Model::Value& v)  const {
	co_yield std::ranges::elements_of(MiniMC::Model::visitValue(*this,v));
      }

      
      template <class Castee>
      auto castPtrToAppropriateInteger(Castee&& v) const   {
        if constexpr (std::is_same_v<Castee, typename Value::Pointer>) {
          return ops.template BitCast<typename Value::I64>(v);
        } else if constexpr (std::is_same_v<Castee, typename Value::Pointer32>) {
          return ops.template BitCast<typename Value::I32>(v);
        } else {
          return v;
        }
      }

      template<class T>
      std::generator<Value> operator() (const T&) const  {
	throw NotImplementedExpr<T> ();
      }
      
      
      template<class T>
      std::generator<Value> operator() (const T& t) const requires (MiniMC::Model::is_root<T>) {
	co_yield ops.create(t);
      }


      std::generator<Value> operator() (const MiniMC::Model::Undef& t) const {
	co_yield std::ranges::elements_of(ops.create(t));
      }
      
      
      template<typename T, MiniMC::Model::TypeID To>
      Value ExecTrunc (T from) const {
	if constexpr (Value::template bitsize<T> () >= MiniMC::Model::BitWidth<To>) {
	  return ops.template Trunc<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }
      
      std::generator<Value> operator() (const MiniMC::Model::TruncExpr& trunc) const  {
	auto visitor = MiniMC::Support::Overload {
	  [&trunc,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
	    switch (trunc.getToType()->getTypeID ()) {
	    case MiniMC::Model::TypeID::I8:
	    return ExecTrunc<T,MiniMC::Model::TypeID::I8> (b);
	    case MiniMC::Model::TypeID::I16:
	    return ExecTrunc<T,MiniMC::Model::TypeID::I16> (b);
	    case MiniMC::Model::TypeID::I32:
	    return ExecTrunc<T,MiniMC::Model::TypeID::I32> (b);
	    case MiniMC::Model::TypeID::I64:
	    return ExecTrunc<T,MiniMC::Model::TypeID::I64> (b);
	    default:
	    std::unreachable();
	    }
	    
	  },
	  MiniMC::Support::Error<Value> {}
	};

	auto transform = [&visitor](Value v) -> Value {return Value::visit (visitor,v);};
	
	for (Value v :  MEval(trunc.getFrom ()))
	  co_yield transform (v);	
      }


      template<typename T, MiniMC::Model::TypeID To>
      Value ExecZExt (T from) const {
	if constexpr (Value::template bitsize<T> () <= MiniMC::Model::BitWidth<To>) {
	  return ops.template ZExt<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }

      std::generator<Value> operator() (const MiniMC::Model::ZExtExpr& zext) const  {
	for (auto v: MEval (zext.getFrom ())) {
	  co_yield  Value::visit (MiniMC::Support::Overload {
	      [&zext,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
		switch (zext.getToType()->getTypeID ()) {
		case MiniMC::Model::TypeID::I8:
		return ExecZExt<T,MiniMC::Model::TypeID::I8> (b);
		case MiniMC::Model::TypeID::I16:
		return ExecZExt<T,MiniMC::Model::TypeID::I16> (b);
		case MiniMC::Model::TypeID::I32:
		return ExecZExt<T,MiniMC::Model::TypeID::I32> (b);
		case MiniMC::Model::TypeID::I64:
		return ExecZExt<T,MiniMC::Model::TypeID::I64> (b);
		default:
		std::unreachable();
		}
		
	      },
	      
		[&zext,this](const typename Value::Bool& b) ->Value  {
		  switch (zext.getToType()->getTypeID()) {
		  case MiniMC::Model::TypeID::I8:
		    return ops.template ZExt<MiniMC::Model::TypeID::I8>(b);
		  case MiniMC::Model::TypeID::I16:
		    return ops.template ZExt<MiniMC::Model::TypeID::I16>(b);
		    break;
		  case MiniMC::Model::TypeID::I32:
		    return ops.template ZExt<MiniMC::Model::TypeID::I32>(b);
		    break;
		  case MiniMC::Model::TypeID::I64:
		    return ops.template ZExt<MiniMC::Model::TypeID::I64>(b);
		    break;
		  default:
		    std::unreachable();
		  }
		},
		MiniMC::Support::Error<Value> {}
	    },
	    v
	    );	
	}
      }

      template<typename T, MiniMC::Model::TypeID To>
      Value ExecSExt (T from) const {
	if constexpr (Value::template bitsize<T> () <= MiniMC::Model::BitWidth<To>) {
	  return ops.template SExt<To> (from);
	}
	else {
	  throw MiniMC::Support::Exception ("Invalid Truncation");
	}
      }
      
      std::generator<Value> operator() (const MiniMC::Model::SExtExpr& sext) const  {
	for (auto v : MEval (sext.getFrom ())) {
	  co_yield Value::visit (MiniMC::Support::Overload {
	      [&sext,this]<typename T>(const T& b) ->Value requires Integer<Value,T> {
		switch (sext.getToType()->getTypeID ()) {
		case MiniMC::Model::TypeID::I8:
		return ExecSExt<T,MiniMC::Model::TypeID::I8> (b);
		case MiniMC::Model::TypeID::I16:
		return ExecSExt<T,MiniMC::Model::TypeID::I16> (b);
		case MiniMC::Model::TypeID::I32:
		return ExecSExt<T,MiniMC::Model::TypeID::I32> (b);
		case MiniMC::Model::TypeID::I64:
		return ExecSExt<T,MiniMC::Model::TypeID::I64> (b);
		default:
		std::unreachable();
		}
		
	      },
		
		[&sext,this](const typename Value::Bool& b) ->Value  {
		  switch (sext.getToType()->getTypeID()) {
		  case MiniMC::Model::TypeID::I8:
		    return ops.template SExt<MiniMC::Model::TypeID::I8>(b);
		  case MiniMC::Model::TypeID::I16:
		    return ops.template SExt<MiniMC::Model::TypeID::I16>(b);
		    break;
		  case MiniMC::Model::TypeID::I32:
		    return ops.template SExt<MiniMC::Model::TypeID::I32>(b);
		    break;
		  case MiniMC::Model::TypeID::I64:
		    return ops.template SExt<MiniMC::Model::TypeID::I64>(b);
		    break;
		  default:
		    std::unreachable();
		  }
		},
		MiniMC::Support::Error<Value> {}
	    },
	    v
	    );
	}
      }

      std::generator<Value> operator() (const MiniMC::Model::LogAndExpr andd) const  {
	for (auto o1 : MEval (andd.op1 ())) {
	  for (auto o2 : MEval (andd.op2 ())) {
	
	  co_yield Value::visit (  MiniMC::Support::Overload {
	      [this] (const Value::Bool v1,Value::Bool v2)->Value  {return ops.BoolAnd (v1,v2);},
		MiniMC::Support::Error<Value>{}
	    },
	    o1,o2
	    );
	  }
	}
      }
      
      std::generator<Value> operator() (const MiniMC::Model::IntToBoolExpr& sext) const  {
	for (auto v : MEval (sext.getFrom ())) {
	  co_yield Value::visit (  MiniMC::Support::Overload {
	      [this]<typename T> (const T v)->Value requires Integer<Value,T> {return ops.IntToBool (v);},
		MiniMC::Support::Error<Value>{}
	    },
	    v
	    );
	}
      }
      
      std::generator<Value> operator() (const MiniMC::Model::IntToPtrExpr& sext) const  {
	for (auto v : MEval (sext.getFrom ())) {
	  co_yield 
	    Value::visit ( MiniMC::Support::Overload {
		[this](const  typename Value::I8& i8)->Value  {
		  auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i8); 
		  return ops.template BitCast<typename Value::Pointer> (extended);
		},
		  [this](const  typename Value::I16& i16)->Value  {
		    auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i16); 
		    return ops.template BitCast<typename Value::Pointer> (extended);
		  },
		  [this] (const  typename Value::I32& i32)->Value  {
		    auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i32); 
		    return ops.template BitCast<typename Value::Pointer> (extended);
		  },
		  [this] (const  typename Value::I64& v)->Value  {
		    return ops.template BitCast<typename Value::Pointer> (v);
		  },
		  MiniMC::Support::Error<Value>{}
	      },
	      v
	      );
	}
      }

      std::generator<Value> operator() (const MiniMC::Model::PtrToIntExpr& sext) const  {
	for (auto v : MEval (sext.getFrom ())) {
	  co_yield Value::visit (  MiniMC::Support::Overload {
	      [this,&sext](const typename Value::Pointer& val)->Value  {
		auto i64 = ops.template BitCast<typename Value::I64> (val);
		switch (sext.getToType ()->getTypeID ()) {
		case MiniMC::Model::TypeID::I8:
		  return ops.template Trunc<MiniMC::Model::TypeID::I8> (i64);
		case MiniMC::Model::TypeID::I16:
		  return ops.template Trunc<MiniMC::Model::TypeID::I16> (i64);
		case MiniMC::Model::TypeID::I32:
		  return ops.template Trunc<MiniMC::Model::TypeID::I32> (i64);
		case MiniMC::Model::TypeID::I64:
		  return i64;
		default:
		  std::unreachable();
		}
	      },
		[this,&sext](const typename Value::Pointer32& val)->Value  {
		  auto i32 = ops.template BitCast<typename Value::I32> (val);
		  
		  switch (sext.getToType ()->getTypeID ()) {
		  case MiniMC::Model::TypeID::I8:
		    return ops.template Trunc<MiniMC::Model::TypeID::I8> (i32);
		  case MiniMC::Model::TypeID::I16:
		    return ops.template Trunc<MiniMC::Model::TypeID::I16> (i32);
		  case MiniMC::Model::TypeID::I32:
		    return i32;
		  case MiniMC::Model::TypeID::I64:
		    return ops.template ZExt<MiniMC::Model::TypeID::I64> (i32);
		  default:
		    std::unreachable();
		  }
		},
		
		MiniMC::Support::Error<Value>{}
	    },
	    v
	    );
	}
      }
      
      std::generator<Value> operator() (const MiniMC::Model::Register& reg) const  {
	co_yield regstore.lookupRegister (reg);
      }
      
      std::generator<Value> operator() (const MiniMC::Model::SymbolicConstant& s) const  {
	co_yield std::visit (
		    MiniMC::Support::Overload {
		      [this](const MiniMC::Model::Register_wptr& r)->Value {return regstore.lookupRegister (*r.lock());},
		      [this,&s](const MiniMC::Model::HeapBlock_wptr&)->Value {return regstore.lookupSymbol (s.getValue());},
		      [this,&s](const MiniMC::Model::Function_wptr& f)->Value {return ops.create((f.lock()->function_ptr()));},
			
		      MiniMC::Support::Error<Value>{}	 
		    },
		    
		    s.getValue().getUserData()
			   );
      }
      
      
      
#define OPSI					\
      X(AddExpr,Add)				\
      X(SubExpr,Sub)				\
      X(MulExpr, Mul)				\
      X(UDivExpr, UDiv)				\
      X(SDivExpr, SDiv)				\
      X(LShlExpr, LShl)				\
      X(AShrExpr, AShr)				\
      X(LShrExpr, LShr)				\
      X(AndExpr, And)				\
      X(OrExpr, Or)				\
      X(XorExpr, Xor)				\
      
#define X(CC,op)							\
      std::generator<Value> operator() (const MiniMC::Model::CC& cc) const  { \
	for (auto l :  MEval (cc.op1())) {				\
	  for (auto r :  MEval (cc.op2())) {				\
	    co_yield Value::visit (MiniMC::Support::Overload {		\
		[this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
		  return ops.op (ll,rr);},				\
		  [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");} \
		  },l,r);}}						\
      }									
      OPSI
#undef X
#undef OPSI

#define OPSI					\
      X(SGtExpr, SGt)			\
      X(UGtExpr, UGt)			\
      X(SGeExpr, SGe)			\
      X(UGeExpr, UGe)			\
      X(SLtExpr, SLt)			\
      X(ULtExpr, ULt)			\
      X(SLeExpr, SLe)			\
      X(ULeExpr, ULe)			\
      X(EqExpr, Eq)			\
      X(NEqExpr, NEq)			\

#define X(CC,op)							\
      std::generator<Value> operator() (const MiniMC::Model::CC& cc) const  { \
	for (auto l : MEval (cc.op1 ())) {				\
	  for( auto r : MEval (cc.op2 ())) {				\
	    co_yield Value::visit (MiniMC::Support::Overload {		\
		[this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
		  return ops.op (ll,rr);},				\
		  [this]<typename T> (T& ll, T& rr) -> Value requires Pointer<Value,T>  { \
		    return ops.op (castPtrToAppropriateInteger<T>(std::forward<T>(ll)),castPtrToAppropriateInteger<T>(std::forward<T>(rr)));}, \
		  MiniMC::Support::Error<Value> {}			\
		  },l,r); } }						\
      }									
      OPSI
#undef X
#undef OPSI     
      
      
#define OPSI								\
      X(LogNotExpr, BoolNegate)						
      
#define X(CC,op)							\
      std::generator<Value> operator() (const MiniMC::Model::CC& cc) const  { \
	for (auto l :   MEval (cc.op1())) {				\
	co_yield Value::visit (MiniMC::Support::Overload {		\
	  [this] (typename Value::Bool& ll) -> Value    {		\
	    return ops.op (ll);						\
	  },								\
	    [](auto&) -> Value {throw MiniMC::Support::Exception ("Error");} \
	    },l								\
	  );	}							\
      }									\
      
OPSI
#undef X
#undef OPSI   
      
      std::generator<Value> operator() (const MiniMC::Model::NotExpr& notex) const  {
	for (auto v : MEval(notex.op1())) {
	  co_yield Value::visit (  MiniMC::Support::Overload {
	      [this]<typename T> (const  T& t) ->Value requires Integer<Value,T>{
		return ops.Not (t);
	      },
		MiniMC::Support::Error<Value>{}
	    },
	    v
	    );
	}
      }
      
      Value::Pointer convertToPointer(const Value& t) const {
	return t.visit (MiniMC::Support::Overload {
	    [] (const typename Value::Pointer& p) {
	      return p;
	    },
	    [this] (const typename Value::Pointer32& p) {
	      return ops.Ptr32ToPtr(p);
	    },
	      MiniMC::Support::Error<typename Value::Pointer>{}
	  }
	  ,t);
      }

      
      
      template<class T,class S,MiniMC::Model::TypeID id>
      T doIntegerLoad (const Value::Pointer& p, const Value::Memory& m) const requires Integer<Value,T> {
	auto s = ops.create(S{0});
	auto shift = ops.create(S{0});

	for (auto b: regstore.loadBytes (p,m,Value::template bytesize<T> ())) {
	  
	  
	  if constexpr (id != MiniMC::Model::TypeID::I8) {
	    auto extended = ops.template ZExt<id> (b);
	    auto shifted = ops.LShl (extended,shift);
	    s = ops.Or (s,shifted);
	    shift = ops.Add (shift,ops.create(S{8}));
	  }

	  else {
	    return b;
	  }
	}
	return s;
      }
      
      std::generator<Value> operator() (const MiniMC::Model::LoadExpr& load) const  {
	for (auto m : MEval (load.mem ())) {
	  for (auto a : MEval (load.addr ())) {
	    auto pointer = convertToPointer (a);
	    co_yield Value::visit (  MiniMC::Support::Overload {
		[this,&load,&pointer] (const typename Value::Memory& m) -> Value{
		  switch (load.getToType()->getTypeID ()) {
		  case MiniMC::Model::TypeID::I8:
		    return  doIntegerLoad<typename Value::I8, MiniMC::Model::I8Integer,MiniMC::Model::TypeID::I8> (pointer,m);
		  case MiniMC::Model::TypeID::I16:
		    return  doIntegerLoad<typename Value::I16, MiniMC::Model::I16Integer,MiniMC::Model::TypeID::I16> (pointer,m);
		  case MiniMC::Model::TypeID::I32:
		    return  doIntegerLoad<typename Value::I32, MiniMC::Model::I32Integer,MiniMC::Model::TypeID::I32> (pointer,m);
		  case MiniMC::Model::TypeID::I64:
		    return  doIntegerLoad<typename Value::I64, MiniMC::Model::I64Integer,MiniMC::Model::TypeID::I64> (pointer,m);
		  case MiniMC::Model::TypeID::Pointer: {
		    typename Value::I64 integer = doIntegerLoad<typename Value::I64, MiniMC::Model::I64Integer,MiniMC::Model::TypeID::I64> (pointer,m);
		    
		    return ops.template BitCast<typename Value::Pointer> (integer);
		  }
		  case MiniMC::Model::TypeID::Pointer32: {
		    typename Value::I32 integer = doIntegerLoad<typename Value::I32, MiniMC::Model::I32Integer,MiniMC::Model::TypeID::I32> (pointer,m);
		    return ops.template BitCast<typename Value::Pointer32> (integer);
		  }
		  case MiniMC::Model::TypeID::Aggregate: {
		    auto aggr = ops.create (MiniMC::Model::AggregateConstant {MiniMC::Util::Array (load.getToType()->getSize())});
		    for (auto [index,b]: std::views::enumerate(regstore.loadBytes (pointer,m,load.getToType()->getSize()))) {
		      aggr = ops.InsertBaseValue (aggr,index,b);
		    }
		    
		    return aggr;
		    
		  }
		  default:
		    //TODO: Make load of pointers  and aggregates use the loadBytes functions
		    throw MiniMC::Support::Exception ("Unsupported load");
		  }
		},
		MiniMC::Support::Error<Value>{}
	      },
	      m
	      );
	  }
	}
      }
      
      
      std::generator<Value> operator() (const MiniMC::Model::StoreExpr& store) const  {
	for ( auto storeto : MEval(store.storeto ())) {
	  for (auto addr : MEval (store.addr ())) {
	    for (auto storee : MEval (store.storee ())) {
	      auto pointer = convertToPointer (addr);
	      co_yield Value::visit(MiniMC::Support::Overload {
		  [this,&pointer]<typename V>(Value::Memory m,const V& t) requires (!Boolean<Value,V> && !MemoryC<Value,V>) {
		    auto ones = ops.create (MiniMC::Model::I64Integer{1});
		    for (auto b :  ops.bytes(t)) {
		      m = regstore.store(m,pointer,b);
		      pointer = ops.PtrAdd (pointer,ones);
		    }
		    return Value{m};
		  },
		  MiniMC::Support::Error<Value> {}
	      },
	      storeto,
	      storee
	      );
	  }
	}
      }
    }
      
      
      
      std::generator<Value> operator() (const MiniMC::Model::PtrAddExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this,&load]<typename ValT>(Value::Pointer& ptr,ValT& skipsize)->Value requires Integer<Value,ValT> {
	    return  ops.PtrAdd(ptr, ops.template ZExt<MiniMC::Model::TypeID::I64>(skipsize));
	    
	  },
	  [this,&load](Value::Pointer& ptr,Value::I64& skipsize)->Value  {
	    return  ops.PtrAdd(ptr, skipsize);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};

	for (auto ptr : MEval (load.ptr())) {
	  for (auto skipsize : MEval (load.skipsize())) {
	      co_yield Value::visit (visitor,ptr,skipsize);  
	    
	  }
	}
	
	
	
      }

      std::generator<Value> operator() (const MiniMC::Model::InsertValueExpr& expr) const  {
	MiniMC::BV64 offset = MiniMC::Model::visitValue(MiniMC::Support::Overload{
	    [](const MiniMC::Model::I16Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I32Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I64Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      MiniMC::Support::Error<MiniMC::BV64> {}
	  },
	  expr.offset()
	  );
	
	for (auto aggr : MEval (expr.aggregate())) {
	  for (auto value : MEval (expr.insertee())) {
	    co_yield Value::visit (MiniMC::Support::Overload {
		[this,offset](const typename Value::Aggregate& aggr,const typename Value::Aggregate& value) ->Value{
		  return ops.template InsertAggregateValue(aggr, offset, value);
		},
		  [this,offset]<typename K>(const typename Value::Aggregate& aggr,const K& value) ->Value requires (!MiniMC::VMT::MemoryC<Value,K>) {
		  return ops.template InsertBaseValue(aggr, offset, value);
		},
		  MiniMC::Support::Error<Value> {}
	      }
	      ,
	      aggr,
	      value);
	    
	  }
	}
      }

      template<class T,class S,MiniMC::Model::TypeID id>
      T doIntegerExtract (const Value::Aggregate& aggr,std::size_t offset) const requires Integer<Value,T> {
	auto s = ops.create(S{0});
	auto shift = ops.create(S{0});

	for (auto b: ops.extractbytes (aggr,offset,Value::template bytesize<T> ())) {
	  
	  
	  if constexpr (id != MiniMC::Model::TypeID::I8) {
	    auto extended = ops.template ZExt<id> (b);
	    auto shifted = ops.LShl (extended,shift);
	    s = ops.Or (s,shifted);
	    shift = ops.Add (shift,ops.create(S{8}));
	  }

	  else {
	    return b;
	  }
	}
	return s;
      }
      
      
      std::generator<Value> operator() (const MiniMC::Model::ExtractValueExpr& expr) const  {
	MiniMC::BV64 offset = MiniMC::Model::visitValue(MiniMC::Support::Overload{
	    [](const MiniMC::Model::I16Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I32Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      [](const MiniMC::Model::I64Integer& value) -> MiniMC::BV64 { return value.getValue(); },
	      MiniMC::Support::Error<MiniMC::BV64> {}
	  },
	  expr.offset()
	  );
	for (auto aggregate : MEval (expr.aggregate())) {
	  typename Value::Aggregate aggr = Value::visit (MiniMC::Support::Overload {
	      [](const typename Value::Aggregate& aggr) {return aggr;},
	      MiniMC::Support::Error<typename Value::Aggregate> {}	
	    },
	    aggregate
	    );
	  switch (expr.getExtractType()->getTypeID()) {
	  case MiniMC::Model::TypeID::I8:
	    co_yield doIntegerExtract<typename Value::I8,MiniMC::Model::I8Integer,MiniMC::Model::TypeID::I8> (aggr,offset);
	    break;
	  case MiniMC::Model::TypeID::I16:
	    co_yield doIntegerExtract<typename Value::I16,MiniMC::Model::I16Integer,MiniMC::Model::TypeID::I16> (aggr,offset);
	    
	    break;
	  case MiniMC::Model::TypeID::I32:
	    co_yield doIntegerExtract<typename Value::I32,MiniMC::Model::I32Integer,MiniMC::Model::TypeID::I32> (aggr,offset);
	    
	    break;
	  case MiniMC::Model::TypeID::I64:
	    co_yield doIntegerExtract<typename Value::I64,MiniMC::Model::I64Integer,MiniMC::Model::TypeID::I64> (aggr,offset);
	    break;
	    
	  case MiniMC::Model::TypeID::Pointer: {
	    auto integer = doIntegerExtract<typename Value::I64,MiniMC::Model::I64Integer,MiniMC::Model::TypeID::I64> (aggr,offset);
	    co_yield ops.template BitCast<typename Value::Pointer> (integer);
	    break;
	  }
	  case MiniMC::Model::TypeID::Pointer32: {
	    auto integer = doIntegerExtract<typename Value::I32,MiniMC::Model::I32Integer,MiniMC::Model::TypeID::I32> (aggr,offset);
	    co_yield ops.template BitCast<typename Value::Pointer32> (integer);
	    break;
	  }
	  case MiniMC::Model::TypeID::Aggregate: {
	    auto aggr = ops.create (MiniMC::Model::AggregateConstant {MiniMC::Util::Array (expr.getExtractType()->getSize())});
	    for (auto [index,b]: std::views::enumerate(ops.extractbytes (aggr,offset,expr.getExtractType()->getSize()))) {
	      aggr = ops.InsertBaseValue (aggr,index,b);
	    }
	    co_yield  aggr;
	    //co_yield ops.ExtractAggregateValue(aggr, offset, expr.getExtractType()->getSize());
	    break;
	  }
	  default:
	    throw MiniMC::Support::Exception("Invalid Extract");
	  }
	}
      }
      

					     
      std::generator<Value> operator() (const MiniMC::Model::PtrSubExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Pointer& ptr,ValT& skipsize)->Value requires Integer<Value,ValT> {
	    return ops.PtrSub(ptr, ops.template ZExt<MiniMC::Model::TypeID::I64> (skipsize));
	    
	  },
	  [this](Value::Pointer& ptr,Value::I64& skipsize)->Value  {
	    return ops.PtrSub(ptr, skipsize);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	for (auto ptr : MEval(load.ptr())) {
	  for (auto skipsize : MEval(load.skipsize ())) {
	    co_yield Value::visit (visitor,ptr,skipsize);
	    
	  }
	}
	  
      }

      std::generator<Value> operator() (const MiniMC::Model::FindSpaceExpr& cc) const  { 

	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Memory& mem,ValT& size)->Value requires Integer<Value,ValT> {
	    return regstore.find_space(mem, ops.template ZExt<MiniMC::Model::TypeID::I64> (size)); 
	  },
	  [this](Value::Memory& mem,Value::I64& size)->Value  {
	    return regstore.find_space(mem, size);
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
      
	for (auto mem : MEval(cc.memory())) {
	  for (auto size : MEval(cc.size ())) {
	    co_yield Value::visit (visitor,mem,size);
	  }
	}
      }

      std::generator<Value> operator() (const MiniMC::Model::AllocExpr& cc) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Memory& mem,Value::Pointer& p, ValT& size)->Value requires Integer<Value,ValT> {
	    return regstore.allocate(mem, p,ops.template ZExt<MiniMC::Model::TypeID::I64> (size)); 
	  },
	  [this](Value::Memory& mem,Value::Pointer& p,Value::I64& size)->Value  {
	    return regstore.allocate(mem, p,size);
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	
	for (auto mem : MEval(cc.memory())) {
	  for (auto pointer : MEval(cc.pointer ())) {
	    for (auto size : MEval(cc.size ())) {
	      co_yield Value::visit (visitor,mem,pointer,size);
	    }
	  }
	}
	
      }


      std::generator<Value> operator() (const MiniMC::Model::FreeExpr& cc) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this](Value::Memory& mem,Value::Pointer& p)->Value  {
	    return regstore.free(mem, p);
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	
	for (auto mem : MEval(cc.memory())) {
	  for (auto pointer : MEval(cc.pointer ())) {
	    co_yield Value::visit (visitor,mem,pointer);
	  }
	}
	
      }	
      
      std::generator<Value> operator() (const MiniMC::Model::CheckFreeExpr& cc) const  { 
	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Memory& mem,Value::Pointer& p, ValT& size)->Value requires Integer<Value,ValT> {
	    return regstore.check_free(mem, p,ops.template ZExt<MiniMC::Model::TypeID::I64> (size)); 
	  },
	  [this](Value::Memory& mem,Value::Pointer& p,Value::I64& size)->Value  {
	    return regstore.check_free(mem, p,size);
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	
	for (auto mem : MEval(cc.memory())) {
	  for (auto pointer : MEval(cc.pointer ())) {
	    for (auto size : MEval(cc.size ())) {
	      co_yield Value::visit (visitor,mem,pointer,size);
	    }
	  }
	}
      }
    
      template<class T>
      std::generator<Value> operator() (const T&) const requires (MiniMC::Model::is_bin_arith<T> || MiniMC::Model::is_bin_cmp<T>) {
	throw NotImplementedExpr<T> ();
      }
      
      
     
      
    private:
      Operations ops;
      const RegStore regstore;
    };
    
    template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    MultiEvaluator<Value,RegStore,Operations> makeEvaluator (RegStore reg, Operations ops) {return MultiEvaluator<Value,RegStore,Operations> (ops,std::move(reg));}
    
    
    template<class Value, Ops<Value> Operations, MemoryOperations<Value> MemControl>
    class Engine {
    public:
      
      Engine (Operations&& ops,MemControl&& memcontrol, const MiniMC::Model::Program& prgm);
      ~Engine ();
      
      template<VMState<Value> VState>
      std::generator<std::shared_ptr<VState>> execute (const MiniMC::Model::InstructionStream&, const VState&, MiniMC::Model::proc_t) ;
      
      template<VMState<Value> VState>
      std::generator<std::shared_ptr<VState>> execute (const MiniMC::Model::Instruction&, const VState&, MiniMC::Model::proc_t) ;

      
      
    private:
      class Impl;
      std::unique_ptr<Impl> _impl;
     };
    
    
    
  }

		
}


#include "minimc/vm/engine_implementation.hpp"

#endif
