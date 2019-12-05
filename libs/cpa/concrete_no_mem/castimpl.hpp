#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/casts.hpp"
#include "support/exceptions.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"

template<MiniMC::Model::InstructionCode i,class F,class T>
struct CastExec {
  static OutRegister execute (const InRegister& left){
    assert(false && "Not Implemented");
  }
};

template<class F,class T>
struct CastExec {
  static OutRegister execute<MiniMC::Model::InstructionCode::ZExt>  (const InRegister& left){
    std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
    *reinterpret_cast<T*> (hh.get ()) = zext<F,T> (left.template get<F> ());
    return OutRegister (hh,sizeof(T));
  }
};

template<class F,class T>
struct CastExec {
  static OutRegister execute<MiniMC::Model::InstructionCode::SExt>  (const InRegister& left){
    std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
    *reinterpret_cast<T*> (hh.get ()) = sext<F,T> (left.template get<F> ());
    return OutRegister (hh,sizeof(T));
  }
};

template<class F,class T>
struct CastExec {
  static OutRegister execute<MiniMC::Model::InstructionCode::Trunc>  (const InRegister& left){
    std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
      *reinterpret_cast<T*> (hh.get ()) = trunc<F,T> (left.template get<F> ());
      return OutRegister (hh,sizeof(T));
  }
};

template<MiniMC::Model::InstructionCode i, class F>
OutRegister RedirectTo (const InRegister& left, MiniMC::Model::Type_ptr& ptr) {
  switch (t->getTypeID ()) {
  case MiniMC::Model::TypeID::Integer: {
	  switch (t->getSize ()) {
	  case 1:
	    return CastExec<i,F,MiniMC::uint8_t>::execute (left);
	  case 2:
	    return CastExec<i,F,MiniMC::uint16_t>::execute (left);
	  case 4:
	    return CastExec<i,F,MiniMC::uint32_t>::execute (left);
	  case 8:
	    return CastExec<i,F,MiniMC::uint64_t>::execute (left);
	  }
  }
  case MiniMC::Model::TypeID::Bool:
    return CastExec<i,F,MiniMC::uint8_t>::execute (left);
  default:
    assert(false && "Not Implemented");
  }
}

template<MiniMC::Model::InstructionCode i>
OutRegister RedirectFrom (const InRegister& left, MiniMC::Model::Type_ptr& from, MiniMC::Model::Type_ptr& to) {
  switch (from->getTypeID ()) {
  case MiniMC::Model::TypeID::Integer: {
    switch (t->getSize ()) {
    case 1:
      return RedirectTo<i,MiniMC::uint8_t> (left,to);
    case 2:
      return RedirectTo<i,MiniMC::uint16_t> (left,to);
    case 4:
      return RedirectTo<i,MiniMC::uint32_t> (left,to);
    case 8:
      return RedirectTo<i,MiniMC::uint64_t> (left,to);
    }
  }
  case MiniMC::Model::TypeID::Bool:
    return RedirectTo<i,MiniMC::uint8_t> (left,to);
  default:
    assert(false && "Not Implemented");
  }
}

