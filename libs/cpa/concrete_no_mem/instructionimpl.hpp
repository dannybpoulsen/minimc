#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"

#include "register.hpp"
#include "stack.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
	  template<MiniMC::Model::InstructionCode,class T>
	  struct TACExec {
		static OutRegister execute (const InRegister& left, const InRegister& right);
	  };
	  
	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Add,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> ()  +right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Sub,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> ()  - right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Mul,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> ()  * right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::UDiv,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = MiniMC::Support::div(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::SDiv,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = MiniMC::Support::idiv(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
	  };
	 
	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Shl,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> () << right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::LShr,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = MiniMC::Support::lshr(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::AShr,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = MiniMC::Support::ashr(left.template get<T> (), right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::And,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> () & right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };

	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Or,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> () | right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };
	  
	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Xor,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> () ^ right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
	  };
	  
	  
	  
	  
	}
  }
}
