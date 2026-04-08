#include "minimc/model/variables.hpp"
#include "minimc/host/operataions.hpp"
#include "minimc/host/casts.hpp"

namespace MiniMC {
  namespace Model {



    
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
      auto op1 = Simplify (i.op1());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.toType()->getTypeID ()) {
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
	      return MiniMC::Model::makeExpr<MiniMC::Model::TruncExpr> (op1,i.toType());} 
	    },
	*op1
	);
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::ZExtExpr& i) const {
      auto op1 = Simplify (i.op1());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.toType()->getTypeID ()) {
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
	      return MiniMC::Model::makeExpr<MiniMC::Model::ZExtExpr> (op1,i.toType());} 
	    },
	*op1
	);
      
    }
    MiniMC::Model::Value_ptr ExprSimplifier::operator() (MiniMC::Model::SExtExpr& i) const {
      auto op1 = Simplify (i.op1 ());
      return MiniMC::Model::visitValue ( MiniMC::Support::Overload {
	  [&i]<typename T>(T& l)->MiniMC::Model::Value_ptr requires MiniMC::Model::Integer<T> {
	    switch (i.toType()->getTypeID ()) {
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
	      return MiniMC::Model::makeExpr<MiniMC::Model::SExtExpr> (op1,i.toType());} 
	    },
	*op1
	);
    
    }

  }
}
