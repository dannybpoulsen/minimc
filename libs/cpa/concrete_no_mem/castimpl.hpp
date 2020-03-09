#include <type_traits>

#include "support/exceptions.hpp"
#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/casts.hpp"
#include "support/exceptions.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {


      template<MiniMC::Model::InstructionCode i,class F,class T,typename C = void>
      struct CastExec {
	static OutRegister execute (const InRegister& left){
	  throw MiniMC::Support::Exception ("Bug");
	}
      };
      
      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::ZExt,F,T,std::enable_if_t<sizeof(F) <= sizeof(T) &&
												      std::is_integral<F>::value &&
												      std::is_integral<T>::value
												      >> {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::endian_safe_cast<T> (hh.get (),sizeof(T)) = MiniMC::Support::zext<F,T> (left.template get<F> ());
	  return OutRegister (hh,sizeof(T));
	}
      };

      template<class F>
      struct CastExec<MiniMC::Model::InstructionCode::IntToBool,F,MiniMC::uint8_t,std::enable_if_t<std::is_integral<F>::value> > {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(MiniMC::uint8_t)]);
	  if (left.template get<F> ()) {
	    *reinterpret_cast<MiniMC::uint8_t*> (hh.get ()) = 1;
	  }
	  else {
	    *reinterpret_cast<MiniMC::uint8_t*> (hh.get ()) = 0;
	  }
	  return OutRegister (hh,sizeof(MiniMC::uint8_t));
	}
      };
	  
      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::SExt,F,T,typename std::enable_if_t<sizeof(F) <= sizeof(T) &&
												      std::is_integral<F>::value &&
												      std::is_integral<T>::value> > {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::endian_safe_cast<T> (hh.get (),sizeof(T)) = MiniMC::Support::sext<F,T> (left.template get<F> ());
	  return OutRegister (hh,sizeof(T));
	}
      };

      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::BoolZExt,F,T,typename std::enable_if_t<sizeof(F) <= sizeof(T) &&
													  std::is_integral<T>::value &&
													  std::is_integral<F>::value >> {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::endian_safe_cast<T> (hh.get (),sizeof(T)) = MiniMC::Support::zext<F,T> (left.template get<F> ());
	  return OutRegister (hh,sizeof(T));
	}
      };

      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::BoolSExt,F,T,typename std::enable_if_t<sizeof(F) <= sizeof(T)&&
											     std::is_integral<T>::value &&
													  std::is_integral<F>::value> > {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::endian_safe_cast<T> (hh.get (),sizeof(T)) = MiniMC::Support::sext<F,T> (left.template get<F> ());
	  return OutRegister (hh,sizeof(T));
	}
      };

      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::Trunc,F,T,std::enable_if_t<
								  sizeof(F) >= sizeof(T) &&
		      std::is_integral<F>::value &&
		      std::is_integral<T>::value
		      >
		      > {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::endian_safe_cast<T> (hh.get (),sizeof(T)) = MiniMC::Support::trunc<F,T> (left.template get<F> ());
	  return OutRegister (hh,sizeof(T));
	}
      };

      template<class F,class T>
      struct CastExec<MiniMC::Model::InstructionCode::BitCast,F,T,typename std::enable_if<sizeof(F) == sizeof(T)>::type> {
	static OutRegister execute  (const InRegister& left){
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  auto buf = reinterpret_cast<const MiniMC::uint8_t*> (left.getMem());
	  std::copy (buf,buf+sizeof(T),hh.get());
	  return OutRegister (hh,sizeof(T));
	}
      };
      
      template<MiniMC::Model::InstructionCode i, class F>
      OutRegister RedirectTo (const InRegister& left, const MiniMC::Model::Type_ptr& t) {
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
	case MiniMC::Model::TypeID::Pointer:
	  return CastExec<i,F,MiniMC::pointer_t>::execute (left);
	  
	case MiniMC::Model::TypeID::Bool:
	  return CastExec<i,F,MiniMC::uint8_t>::execute (left);
	default:
	  throw MiniMC::Support::Exception ("Bug");
	  
	}
      }

      template<MiniMC::Model::InstructionCode i>
      OutRegister RedirectFrom (const InRegister& left, const MiniMC::Model::Type_ptr& from, const MiniMC::Model::Type_ptr& to) {
	switch (from->getTypeID ()) {
	case MiniMC::Model::TypeID::Integer: {
	  switch (from->getSize ()) {
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
	case MiniMC::Model::TypeID::Pointer:
	  return RedirectTo<i,MiniMC::pointer_t> (left,to);
	case MiniMC::Model::TypeID::Bool:
	  return RedirectTo<i,MiniMC::uint8_t> (left,to);
	default:
	  throw MiniMC::Support::Exception ("Bug");
	}
      }


    }
  }
}
