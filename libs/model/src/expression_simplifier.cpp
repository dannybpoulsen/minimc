#include "minimc/model/variables.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/host/casts.hpp"

namespace MiniMC {
  namespace Model {

#define OPS					\
    X(Add)					\
    X(Sub)					\
    X(UDiv)					\
    X(SDiv)					\
    X(Mul)					\
    X(LShl)					\
    X(LShr)					\
    X(AShr)					\
    X(And)					\
    X(Or)					\
    X(Xor)					\

    
#define X(OP)								\
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::OP##Expr& i) const { \
      auto op1 = Simplify (i.op1());					\
    auto op2 = Simplify (i.op2());					\
									\
    return MiniMC::Model::visitValue ( MiniMC::Support::Overload {	\
	[]<typename T>(T& l, T& r)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> { \
      return T::make (MiniMC::Host::Op<MiniMC::Host::TAC::OP> (l.getValue(),r.getValue())); \
    },									\
											      [&op1,&op2](auto&, auto&)->MiniMC::Model::Value_ptr{return MiniMC::Model::makeExpr<MiniMC::Model::OP##Expr> (op1,op2);} \
											      }, \
      *op1,*op2);				\
		 \
    }		 \

    OPS
#undef OPS    
#undef X
    
    template <class T, class P>
    P trunc_checked(const T& t) {
      if constexpr (sizeof(T) == sizeof(P))
	return t;
      else if constexpr (sizeof(T) < sizeof(P)) {
	
	throw MiniMC::Support::Exception ("Improper truncation");
      }
      else {
	return MiniMC::Host::trunc<T,P> (t);
      }
    }

    template <class T, class P>
    P zext_checked(const T& t) {
      if constexpr (sizeof(T) == sizeof(P))
	return t;
      else if constexpr (sizeof(T) > sizeof(P)) {
	throw MiniMC::Support::Exception ("Improper truncation");
      }
      else {
	return MiniMC::Host::zext<T,P> (t);
      }
    }
    
    template <class T, class P>
    P sext_checked(const T& t) {
      if constexpr (sizeof(T) == sizeof(P))
	return t;
      else if constexpr (sizeof(T) > sizeof(P)) {
	throw MiniMC::Support::Exception ("Improper truncation");
      }
      else {
	return MiniMC::Host::sext<T,P> (t);
      }
    }
 
    
    MiniMC::Model::Value_ptr
    ExprSimplifier::operator()(MiniMC::Model::TruncExpr& i) const {
      auto op1 = Simplify (i.getFrom());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.getToType()->getTypeID ()) {
	    case MiniMC::Model::TypeID::I8:
	    return MiniMC::Model::I8Integer::make (trunc_checked<typename T::underlying_type,MiniMC::Model::I8Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I16:
	    return MiniMC::Model::I16Integer::make (trunc_checked<typename T::underlying_type,MiniMC::Model::I16Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I32:
	    return MiniMC::Model::I32Integer::make (trunc_checked<typename T::underlying_type,MiniMC::Model::I32Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I64:
	    return MiniMC::Model::I64Integer::make (trunc_checked<typename T::underlying_type,MiniMC::Model::I64Integer::underlying_type> (l.getValue ()));
	    default:
	    throw MiniMC::Support::Exception ("Weird Truncation");
	    }
	    return nullptr;
	    
	  },								
	    [&op1,&i](auto&)->MiniMC::Model::Value_ptr{
	      return MiniMC::Model::makeExpr<MiniMC::Model::TruncExpr> (op1,i.getToType());} 
	    },
	*op1
	);
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::ZExtExpr& i) const {
      auto op1 = Simplify (i.getFrom());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.getToType()->getTypeID ()) {
	    case MiniMC::Model::TypeID::I8:
	    return MiniMC::Model::I8Integer::make (zext_checked<typename T::underlying_type,MiniMC::Model::I8Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I16:
	    return MiniMC::Model::I16Integer::make (zext_checked<typename T::underlying_type,MiniMC::Model::I16Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I32:
	    return MiniMC::Model::I32Integer::make (zext_checked<typename T::underlying_type,MiniMC::Model::I32Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I64:
	    return MiniMC::Model::I64Integer::make (zext_checked<typename T::underlying_type,MiniMC::Model::I64Integer::underlying_type> (l.getValue ()));
	    default:
	    throw MiniMC::Support::Exception ("Weird Extenstion");
	    }
	    return nullptr;
	    
	  },								
	    [&op1,&i](auto&)->MiniMC::Model::Value_ptr{
	      return MiniMC::Model::makeExpr<MiniMC::Model::ZExtExpr> (op1,i.getToType());} 
	    },
	*op1
	);
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::SExtExpr& i) const {
      auto op1 = Simplify (i.getFrom());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.getToType()->getTypeID ()) {
	    case MiniMC::Model::TypeID::I8:
	    return MiniMC::Model::I8Integer::make (sext_checked<typename T::underlying_type,MiniMC::Model::I8Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I16:
	    return MiniMC::Model::I16Integer::make (sext_checked<typename T::underlying_type,MiniMC::Model::I16Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I32:
	    return MiniMC::Model::I32Integer::make (sext_checked<typename T::underlying_type,MiniMC::Model::I32Integer::underlying_type> (l.getValue ()));
	    case MiniMC::Model::TypeID::I64:
	    return MiniMC::Model::I64Integer::make (sext_checked<typename T::underlying_type,MiniMC::Model::I64Integer::underlying_type> (l.getValue ()));
	    default:
	    throw MiniMC::Support::Exception ("Weird Extenstion");
	    }
	    
	  },								
	    [&op1,&i](auto&)->MiniMC::Model::Value_ptr{
	      return MiniMC::Model::makeExpr<MiniMC::Model::SExtExpr> (op1,i.getToType());} 
	    },
	*op1
	);
    
    }

#define OPS								\
      X(SGt,SGT)							\
      X(UGt,UGT)							\
      X(SGe,SGE)							\
      X(UGe,UGE)								\
      X(SLt,SLT)							\
      X(ULt,ULT)							\
      X(SLe,SLE)							\
      X(ULe,ULE)							\
      X(Eq,EQ)								\
      X(NEq,NEQ)								\

#define X(OP,COM)								\
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::OP##Expr& i) const { \
      auto op1 = Simplify (i.op1());					\
      auto op2 = Simplify (i.op2());					\
      									\
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {	\
	  []<typename T>(T& l, T& r)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> { \
	    return MiniMC::Model::Bool::make (MiniMC::Host::Op<MiniMC::Host::CMP::COM> (l.getValue(),r.getValue())); \
	  },								\
	    [&op1,&op2](auto&, auto&)->MiniMC::Model::Value_ptr{return MiniMC::Model::makeExpr<MiniMC::Model::OP##Expr> (op1,op2);} \
	    },								\
	*op1,*op2);							\
      									\
    }
    
    OPS
#undef X
#undef OPS    

    
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LogAndExpr& i) const {
      auto l = Simplify (i.op1());
      auto r = Simplify (i.op2());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Bool& ll,MiniMC::Model::Bool& rr)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Bool::make (ll.getValue() && rr.getValue());
	  },
	  [&l,&r](auto&, auto&) {
	    return MiniMC::Model::makeExpr<MiniMC::Model::LogAndExpr> (l,r);
	  }
	    },
	*l,*r);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::NotExpr& i) const {
      return i.shared_from_this();
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LogNotExpr& i) const {return i.shared_from_this();}

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrAddExpr& i) const {
      auto ptr = Simplify (i.ptr());
      auto skipsize = Simplify (i.skipsize());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Pointer& p,MiniMC::Model::I64Integer& skip)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Pointer::make (p.getValue().add (skip.getValue ()));
	    },
	    [&ptr,&skipsize](auto&, auto&) {
	      return MiniMC::Model::makeExpr<MiniMC::Model::PtrAddExpr> (ptr,skipsize);}
	    },
	*ptr,*skipsize);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrSubExpr& i) const {
        auto ptr = Simplify (i.ptr());
      auto skipsize = Simplify (i.skipsize());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [](MiniMC::Model::Pointer& p,MiniMC::Model::I64Integer& skip)->MiniMC::Model::Value_ptr  {
	    return MiniMC::Model::Pointer::make (p.getValue().sub (skip.getValue ()));
	    },
	    [&ptr,&skipsize](auto&, auto&) {
	      return MiniMC::Model::makeExpr<MiniMC::Model::PtrSubExpr> (ptr,skipsize);}
	    },
	*ptr,*skipsize);
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::ExtractValueExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::ExtractValueExpr> (Simplify (i.aggregate()),Simplify (i.offset()),i.getExtractType());
    }

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::AllocExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::AllocExpr> (Simplify (i.memory()),Simplify (i.pointer()),Simplify(i.size()));
    }

    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::FreeExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::FreeExpr> (Simplify (i.memory()),Simplify (i.pointer()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::CheckFreeExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::CheckFreeExpr> (Simplify (i.memory()),Simplify (i.pointer()),Simplify(i.size()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::FindSpaceExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::FindSpaceExpr> (Simplify (i.memory()),Simplify(i.size()));
    }
    
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::InsertValueExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::InsertValueExpr> (Simplify (i.aggregate()),Simplify(i.offset()),Simplify (i.insertee()));
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::BitCastExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::BitCastExpr> (Simplify (i.getFrom()),i.getToType());
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::PtrToIntExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::PtrToIntExpr> (Simplify (i.getFrom()),i.getToType());
     
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::IntToPtrExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::IntToPtrExpr> (Simplify (i.getFrom()),i.getToType());
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::StoreExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::StoreExpr> (Simplify(i.storeto()),Simplify (i.addr()),Simplify(i.storee()));
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::LoadExpr& i) const {
      return MiniMC::Model::makeExpr<MiniMC::Model::LoadExpr> (Simplify(i.mem()),Simplify (i.addr()),i.getToType());
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::IntToBoolExpr& i) const {return i.shared_from_this();}
    
    
  }
}
