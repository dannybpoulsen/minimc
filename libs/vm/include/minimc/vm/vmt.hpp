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
    
    
    template<class Eval,class T>
    concept RegisterStore = requires (MiniMC::Model::Symbol s, const MiniMC::Model::Register& reg, const Eval& ceval, Eval& eval,  T&& t, const T::Pointer p,const T::Memory& mem,const MiniMC::Model::Type& ty, const T& value) {
      {ceval.lookupRegister (reg)} -> std::convertible_to<T>;
      {ceval.lookupSymbol (s)} -> std::convertible_to<T>;
      
      {eval.saveValue (reg,std::move(t))};
      {ceval.load(p,mem,ty)}->std::convertible_to<T>;
      {ceval.store(mem,p,value)}->std::convertible_to<T>;
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

    
    enum class  Status{
      Ok,
      AssumeViolated,
      AssertViolated,
      UnsupportedOperation
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
      ValueLookupable<State,V> && requires (State& s,typename V::Bool&& v) {
      {s.getPathform()}->std::convertible_to<typename V::Bool>;
      {s.setPathform(std::move(v))};
      {s.constraint_solver ()} -> ConstraintSolver<V>;
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
    
    
    /*template<class Value,RegisterStore<Value> RegStore,Ops<Value> Operations>
    class Evaluator {
    public:
      Evaluator (Operations ops, const RegStore regstore) : ops(ops),regstore(std::move(regstore)) {}
      
      Value Eval (const MiniMC::Model::Value& v)  const {
	return  MiniMC::Model::visitValue<Value>(*this,v);
      }

      std::generator<Value> MEval (const MiniMC::Model::Value& v)  const {
	co_yield MiniMC::Model::visitValue<Value>(*this,v);
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
      Value operator() (const T&) const  {
	throw MiniMC::Support::Exception ("Not implemented");
      }
      
      
      template<class T>
      Value operator() (const T& t) const requires (MiniMC::Model::is_root<T>) {
	return  ops.create(t);
      }

      template<class T>
      Value operator() (const MiniMC::Model::Undef& t) const  {
	for (auto t : ops.create(t))
	  return t;
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

      Value operator() (const MiniMC::Model::TruncExpr& trunc) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
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
	  },
	  Eval(trunc.getFrom ())
	  );

	return res;
	
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

      Value operator() (const MiniMC::Model::ZExtExpr& zext) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
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
	  Eval(zext.getFrom ())
	  );

	return res;
	
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
      
      Value operator() (const MiniMC::Model::SExtExpr& sext) const  {
	Value res = Value::visit (MiniMC::Support::Overload {
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
	  Eval(sext.getFrom ())
	  );

	return res;
	
      }

      Value operator() (const MiniMC::Model::IntToBoolExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const T v)->Value requires Integer<Value,T> {return ops.IntToBool (v);},
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (sext.getFrom ())
	  );
      }
      
      Value operator() (const MiniMC::Model::IntToPtrExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const  typename Value::I8& i8)->Value  {
	      auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i8); 
	      return ops.template BitCast<typename Value::Pointer> (extended);
	    },
	    [this]<typename T> (const  typename Value::I16& i16)->Value  {
	      auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i16); 
	      return ops.template BitCast<typename Value::Pointer> (extended);
	    },
	    [this]<typename T> (const  typename Value::I32& i32)->Value  {
	      auto extended = ops.template ZExt<MiniMC::Model::TypeID::I64> (i32); 
	      return ops.template BitCast<typename Value::Pointer> (extended);
	    },
	    [this]<typename T> (const  typename Value::I64& v)->Value  {
	      return ops.template BitCast<typename Value::Pointer> (v);
	    },
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (sext.getFrom ())
	  );
      }

      Value operator() (const MiniMC::Model::PtrToIntExpr& sext) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this,&sext](const typename Value::Pointer& val)->Value  {
	      auto i64  = ops.template BitCast<typename Value::I64> (val);
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
	  Eval (sext.getFrom ())
	  );
      }
      
      Value operator() (const MiniMC::Model::Register& reg) const  {
	return regstore.lookupRegister (reg);
      }
      
      Value operator() (const MiniMC::Model::SymbolicConstant& s) const  {
	return std::visit (
		    MiniMC::Support::Overload {
		      [this](const MiniMC::Model::Register_wptr& r)->Value {return regstore.lookupRegister (*r.lock());},
		      [this,&s](const MiniMC::Model::HeapBlock_wptr&)->Value {return regstore.lookupSymbol (s.getValue());},
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
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1());					\
	auto r = Eval (cc.op2());					\
      return Value::visit (MiniMC::Support::Overload {			\
      [this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
	return ops.op (ll,rr);},					\
      [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");} \
    },l,r);								\
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
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1 ());					\
	auto r = Eval (cc.op2 ());					\
	return Value::visit (MiniMC::Support::Overload {		\
	    [this]<typename T> (T& ll, T& rr) -> Value requires Integer<Value,T>  { \
	      return ops.op (ll,rr);},					\
	      [this]<typename T> (T& ll, T& rr) -> Value requires Pointer<Value,T>  { \
		return ops.op (castPtrToAppropriateInteger<T>(std::forward<T>(ll)),castPtrToAppropriateInteger<T>(std::forward<T>(rr)));}, \
	      [](auto&, auto& ) -> Value {throw MiniMC::Support::Exception ("Error");} \
	      },l,r);							\
      }									
      OPSI
#undef X
#undef OPSI     
      
      
#define OPSI								\
      X(LogNotExpr, BoolNegate)						
      
#define X(CC,op)							\
      Value operator() (const MiniMC::Model::CC& cc) const  {		\
	auto l = Eval (cc.op1());					\
	return Value::visit (MiniMC::Support::Overload {		\
	  [this] (typename Value::Bool& ll) -> Value    {		\
	    return ops.op (ll);					\
	  },								\
	  [](auto&) -> Value {throw MiniMC::Support::Exception ("Error");} \
	    },l								\
	  );								\
      }									\
      
OPSI
#undef X
#undef OPSI   
      
      Value operator() (const MiniMC::Model::NotExpr& notex) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this]<typename T> (const  T& t) ->Value requires Integer<Value,T>{
	      return ops.Not (t);
	    },
	    MiniMC::Support::Error<Value>{}
	  },
	  Eval (notex.op1 ())
	  );
      }

      
      Value operator() (const MiniMC::Model::LoadExpr& load) const  {
	return Value::visit (  MiniMC::Support::Overload {
	    [this,&load] (const typename Value::Memory& m,const  typename Value::Pointer& p) {
	      return regstore.load (p,m,*load.getToType());
	    },
	    [this,&load] (const typename Value::Memory& m,const  typename Value::Pointer32& p) {
	      return regstore.load (ops.Ptr32ToPtr (p),m,*load.getToType());
	    },
	      MiniMC::Support::Error<Value>{}
	  },
	  Eval (load.mem()),
	  Eval (load.addr ())
	  );
      }

      Value operator() (const MiniMC::Model::StoreExpr
			& store) const  {
	return Value::visit(MiniMC::Support::Overload {
	    [this]<typename V>(const typename Value::Memory& m,const typename Value::Pointer& addr,const V& t) requires (!Boolean<Value,V> && !MemoryC<Value,V>) {
	      return  regstore.store(m,addr, t);
	    },
	    [this]<typename V>(const typename Value::Memory& m,const typename Value::Pointer32& addr,const V& t) requires (!Boolean<Value,V> && !MemoryC<Value,V>) {
	      return  regstore.store(m,ops.Ptr32ToPtr(addr), t);
	    },
	    MiniMC::Support::Error<Value> {}
	  },
	  Eval(store.storeto()),
	  Eval(store.addr ()),
	  Eval(store.storee())
	  );
      }

      
      
      Value operator() (const MiniMC::Model::PtrAddExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this,&load]<typename ValT>(Value::Pointer& ptr,ValT& skipsize)->Value requires Integer<Value,ValT> {
	    return  ops.PtrAdd(ptr, ops.template ZExt<MiniMC::Model::TypeID::I64> (skipsize));
	    
	  },
	  [this,&load](Value::Pointer& ptr,Value::I64& skipsize)->Value  {
	    return  ops.PtrAdd(ptr, skipsize);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	return Value::visit (visitor,Eval(load.ptr()),Eval(load.skipsize()));
      }

      Value operator() (const MiniMC::Model::PtrSubExpr& load) const  {
	auto visitor = MiniMC::Support::Overload {
	  [this]<typename ValT>(Value::Pointer& ptr,ValT& skipsize)->Value requires Integer<Value,ValT> {
	    return ops.PtrSub(ptr, skipsize);
	    
	  },
	  MiniMC::Support::Error<Value>{}
	};
	
	return Value::visit (visitor,Eval(load.ptr()),Eval(load.skipsize()));
      }
      
      template<class T>
      Value operator() (const T&) const requires (MiniMC::Model::is_bin_arith<T> || MiniMC::Model::is_bin_cmp<T>) {
	throw MiniMC::Support::Exception ("Not Implemented");
      }
      
      
      
      
    private:
      Operations ops;
      const RegStore regstore;
    };
    */
    
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
	co_yield std::ranges::elements_of(MiniMC::Model::visitValue<std::generator<Value>>(*this,v));
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
	throw MiniMC::Support::Exception ("Not implemented");
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
      
      
      std::generator<Value> operator() (const MiniMC::Model::LoadExpr& load) const  {
	for (auto m : MEval (load.mem ())) {
	  for (auto a : MEval (load.addr ())) {
	    co_yield Value::visit (  MiniMC::Support::Overload {
		[this,&load] (const typename Value::Memory& m,const  typename Value::Pointer& p) {
		  return regstore.load (p,m,*load.getToType());
		},
		  [this,&load] (const typename Value::Memory& m,const  typename Value::Pointer32& p) {
		    return regstore.load (ops.Ptr32ToPtr (p),m,*load.getToType());
		  },
		  MiniMC::Support::Error<Value>{}
	      },
	      m,
	      a
	      );
	  }
	}
      }
      
      
      std::generator<Value> operator() (const MiniMC::Model::StoreExpr& store) const  {
	for ( auto storeto : MEval(store.storeto ())) {
	  for (auto addr : MEval (store.addr ())) {
	    for (auto storee : MEval (store.storee ())) {
	      co_yield Value::visit(MiniMC::Support::Overload {
		[this]<typename V>(const typename Value::Memory& m,const typename Value::Pointer& addr,const V& t) requires (!Boolean<Value,V> && !MemoryC<Value,V>) {
		 
		  return  regstore.store(m,addr, t);
		},
		  [this]<typename V>(const typename Value::Memory& m,const typename Value::Pointer32& addr,const V& t) requires (!Boolean<Value,V> && !MemoryC<Value,V>) {
		  return  regstore.store(m,ops.Ptr32ToPtr(addr), t);
		},
		  MiniMC::Support::Error<Value> {}
	      },
	      storeto,
	      addr,
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
	MiniMC::BV64 offset = MiniMC::Model::visitValue<MiniMC::BV64>(MiniMC::Support::Overload{
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

      std::generator<Value> operator() (const MiniMC::Model::ExtractValueExpr& expr) const  {
	MiniMC::BV64 offset = MiniMC::Model::visitValue<MiniMC::BV64>(MiniMC::Support::Overload{
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
	      co_yield ops.template ExtractBaseValue<typename Value::I8>(aggr, offset);
	      break;
	    case MiniMC::Model::TypeID::I16:
	      co_yield ops.template ExtractBaseValue<typename Value::I16>(aggr, offset);
	      break;
	    case MiniMC::Model::TypeID::I32:
	      co_yield ops.template ExtractBaseValue<typename Value::I32>(aggr, offset);
	      break;
	    case MiniMC::Model::TypeID::I64:
	      co_yield ops.template ExtractBaseValue<typename Value::I64>(aggr, offset);
	      break;
	      
	    case MiniMC::Model::TypeID::Pointer:
	      co_yield ops.template ExtractBaseValue<typename Value::Pointer>(aggr, offset);
	      break;
	    case MiniMC::Model::TypeID::Aggregate:
	      co_yield ops.ExtractAggregateValue(aggr, offset, expr.getExtractType()->getSize());
	      break;
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
      
      template<class T>
      std::generator<Value> operator() (const T&) const requires (MiniMC::Model::is_bin_arith<T> || MiniMC::Model::is_bin_cmp<T>) {
	throw MiniMC::Support::Exception ("Not Implemented");
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
      struct Result {
	Result (Value::Bool assertions, Value::Bool assumes, Status status) : assertions(assertions),assumes(assumes),status(status) {}
	Value::Bool assertions;
	Value::Bool assumes;
	Status status;
      };
      
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
