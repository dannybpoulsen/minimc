#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      template<MiniMC::Model::InstructionCode i,class T>
	  struct TACExec {
	static OutRegister execute (const InRegister& left, const InRegister& right){
	  assert(false && "Not Implemented");
	}
      };
	  
	  template<class T>
	  struct TACExec<MiniMC::Model::InstructionCode::Add,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  *reinterpret_cast<T*> (hh.get ()) = left.template get<T> () + right.template get<T> ();
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
	  
	  template<MiniMC::Model::InstructionCode opc>
	  OutRegister TACRedirect (const InRegister& left, const InRegister& right, MiniMC::Model::Type_ptr& t ) {
		  switch (t->getTypeID ()) {
		  case MiniMC::Model::TypeID::Integer: {
			switch (t->getSize ()) {
			case 1:
			  return TACExec<opc,MiniMC::uint8_t>::execute (left,right);
			case 2:
			  return TACExec<opc,MiniMC::uint16_t>::execute (left,right);
			case 4:
			  return TACExec<opc,MiniMC::uint32_t>::execute (left,right);
			case 8:
			  return TACExec<opc,MiniMC::uint64_t>::execute (left,right);
			}
		  }
		  case MiniMC::Model::TypeID::Bool:
			return TACExec<opc,MiniMC::uint8_t>::execute (left,right);
		  default:
			assert(false && "Not Implemented");
		  }
	  }
	 	  
	  template<MiniMC::Model::InstructionCode opc,class S = void>
	  struct ExecuteInstruction {
		void execute (MiniMC::CPA::ConcreteNoMem::Stack& s, MiniMC::Model::Instruction& )  {
		  //Skip
		}
	  };

	  template<MiniMC::Model::InstructionCode opc>
	  struct ExecuteInstruction<opc,typename std::enable_if<MiniMC::Model::InstructionData<opc>::isTAC>::type> {
		void execute (MiniMC::CPA::ConcreteNoMem::Stack& s, MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<opc> helper (inst);
		  auto l = s.load (helper.getLeftOp ());
		  auto r = s.load (helper.getRightOp ());
		  auto res = TACRedirect (l,r,helper.getLeftOp ()->getType ());
		  s.save (res,helper.getResult ());
		  
		}
	  };

	  
      
    }
  }
}
