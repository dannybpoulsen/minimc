#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"
#include "support/exceptions.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"
#include "compimpl.hpp"

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
      struct TACRedirect{
	using i8 = TACExec<opc,MiniMC::uint8_t>;
	using i16 = TACExec<opc,MiniMC::uint16_t>;
	using i32 = TACExec<opc,MiniMC::uint32_t>;
	using i64 = TACExec<opc,MiniMC::uint64_t>;
	using boolean = TACExec<opc,MiniMC::uint8_t>;
      };
      
      template<class T>
      OutRegister TypeRedirect (const InRegister& left, const InRegister& right, const MiniMC::Model::Type_ptr& t ) {
	switch (t->getTypeID ()) {
	case MiniMC::Model::TypeID::Integer: {
	  switch (t->getSize ()) {
	  case 1:
	    return T::i8::execute (left,right);
	  case 2:
	    return T::i16::execute (left,right);
	  case 4:
	    return T::i32::execute (left,right);
	  case 8:
	    return T::i64::execute (left,right);
	  }
	}
	case MiniMC::Model::TypeID::Bool:
	  return T::boolean::execute (left,right);
	default:
	  assert(false && "Not Implemented");
	}
      }
	 	  
      template<MiniMC::Model::InstructionCode opc,class S = void>
      struct ExecuteInstruction {
	static void execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails&, const MiniMC::Model::Instruction& )  {
	}
      };

      class RegisterLoader {
      public:
	RegisterLoader (State::StackDetails details, const MiniMC::Model::Value_ptr& ptr) {
	  if(ptr->isConstant ()) {
	    auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (ptr);
	    if (!constant->isAggregate ()) {
	      auto iconstant = std::static_pointer_cast<MiniMC::Model::IntegerConstant> (ptr);
	      std::unique_ptr<MiniMC::uint8_t[]> buffer  =  nullptr;
	      std::size_t size;
	      
	      switch (ptr->getType()->getSize()) {
	      case 1:
		setUpInteger<MiniMC::uint8_t> (buffer,size,*iconstant);
		break;
	      case 2:
		setUpInteger<MiniMC::uint16_t> (buffer,size,*iconstant);
		break;
	      case 4:
		setUpInteger<MiniMC::uint32_t> (buffer,size,*iconstant);
		break;
	      case 8:
		setUpInteger<MiniMC::uint64_t> (buffer,size,*iconstant);
		break;
	      }
	      reg = std::make_unique<InnerStateConst> (buffer,size);
	      
	    }
	    else {
	      throw MiniMC::Support::Exception ("Constants cannot be created right now");
	    }
	  }
	  else {
	    auto regi = (ptr->isGlobal() ? details.gstack : details.stack).load (std::static_pointer_cast<MiniMC::Model::Variable> (ptr), ptr->isGlobal() ? details.galloc : details.alloc);
	    reg = std::make_unique<InnerStateIn> (regi);
	  }
	}
	
	const InRegister& getRegister () const {return reg->getReg ();}
	
      private:
	template<class T>
	void setUpInteger (std::unique_ptr<MiniMC::uint8_t[]>& buffer, std::size_t& s, MiniMC::Model::IntegerConstant& constant) {
	  buffer.reset(new MiniMC::uint8_t [sizeof(T)]);
	  s = sizeof(T);
	  reinterpret_cast<T&> (*buffer.get()) =  static_cast<T> (constant.getValue ());
	}
	
	struct InnerState {
	  virtual const InRegister& getReg () const = 0;
	};

	struct InnerStateIn : public InnerState {
	  InnerStateIn (InRegister &reg ) : reg(reg) {}
	  virtual const InRegister& getReg () const {return reg;}
	  InRegister reg;
	};

	struct InnerStateConst : public InnerState {
	  InnerStateConst ( std::unique_ptr<MiniMC::uint8_t[]>& buffer, std::size_t size) : reg(buffer,size) {
	    
	  }
	  virtual const InRegister& getReg () const {return reg;}
	  ConstRegister reg;
	};
	
	std::unique_ptr<InnerState> reg;
      };

      template<class Register> 
      inline void doSave (State::StackDetails& st, MiniMC::Model::Variable_ptr var, Register& reg) {
	if (var->isGlobal()) {
	  st.gstack.save (reg,var,st.galloc);
	}
	else {
	  st.stack.save (reg,var,st.alloc);
	}
      }
      
      template<MiniMC::Model::InstructionCode opc>
      struct ExecuteInstruction<opc,typename std::enable_if<MiniMC::Model::InstructionData<opc>::isTAC>::type> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
			     const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<opc> helper (inst);
	  RegisterLoader l (st,helper.getLeftOp ());
	  RegisterLoader r (st,helper.getRightOp ());
	  auto res = TypeRedirect<TACRedirect<opc>> (l.getRegister(),r.getRegister(),helper.getLeftOp ()->getType ());
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),res);
	  
	}
      };

      
      
      template<MiniMC::Model::InstructionCode opc>
      struct ExecuteInstruction<opc,typename std::enable_if<MiniMC::Model::InstructionData<opc>::isComparison>::type> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<opc> helper (inst);
	  RegisterLoader l (st,helper.getLeftOp ());
	  RegisterLoader r (st,helper.getRightOp ());
	  auto res = TypeRedirect<CMPRedirect<opc>> (l.getRegister(),r.getRegister(),helper.getLeftOp ()->getType ());
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),res);
	  
	  
	}
      };

      
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Assign,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assign> helper (inst);
	  RegisterLoader val (st,helper.getValue ());
	  auto resultvar = helper.getResult ();
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),val.getRegister());
	  
	}
      };
      
      
    }
  }
}
