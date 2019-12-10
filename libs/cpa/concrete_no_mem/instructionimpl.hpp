#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"
#include "support/exceptions.hpp"
#include "support/pointer.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"
#include "compimpl.hpp"
#include "castimpl.hpp"

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
	RegisterLoader (const State::StackDetails& details, const MiniMC::Model::Value_ptr& ptr) {
	  if(ptr->isConstant ()) {
	    auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (ptr);
	    if (!constant->isAggregate ()) {
	      
	      auto iconstant = std::dynamic_pointer_cast<MiniMC::Model::IntegerConstant> (ptr);
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
	    auto regi = (ptr->isGlobal() ? details.gstack : details.stack).load (std::static_pointer_cast<MiniMC::Model::Variable> (ptr));
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
	  st.gstack.save (reg,var);
	}
	else {
	  st.stack.save (reg,var);
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
      struct ExecuteInstruction<opc,typename std::enable_if<MiniMC::Model::InstructionData<opc>::isCast>::type> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
			     const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<opc> helper (inst);
	  auto& castee = helper.getCastee ();
	  auto& res =  helper.getResult ();
	  RegisterLoader l (st,castee);
	  auto& casteeval =  l.getRegister ();
	  
	  auto resval = RedirectFrom<opc> (l.getRegister (), castee->getType(), res->getType ());
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
	  
	}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::IntToPtr,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
			     const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::IntToPtr> helper (inst);
	  auto& castee = helper.getCastee ();
	  auto& res =  helper.getResult ();
	  assert(castee->getType ()->getSize() == res->getType ()->getSize());
	  std::size_t size =  castee->getType ()->getSize();
	    
	  RegisterLoader l (st,castee);
	  auto& casteeval =  l.getRegister ();

	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[size]);
	  auto buf = reinterpret_cast<const MiniMC::uint8_t*> (casteeval.getMem());
	  std::copy (buf,buf+size,hh.get());
	  OutRegister resval (hh,size);
	  
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
	  
	}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::PtrToInt,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
			     const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::PtrToInt> helper (inst);
	  auto& castee = helper.getCastee ();
	  auto& res =  helper.getResult ();
	  assert(castee->getType ()->getSize() == res->getType ()->getSize());
	  std::size_t size =  castee->getType ()->getSize();
	    
	  RegisterLoader l (st,castee);
	  auto& casteeval =  l.getRegister ();

	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[size]);
	  auto buf = reinterpret_cast<const MiniMC::uint8_t*> (casteeval.getMem());
	  std::copy (buf,buf+size,hh.get());
	  OutRegister resval (hh,size);
	  
	  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
	  
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

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::RetVoid,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  { 
	  assert(!st.stack.isModified());
	  assert(!st.stack.getData().ret);
	  st.pop_frame();
	}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Call,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Call> helper (inst);
	  //Missing evaluation of parameters
	  RegisterLoader loader (st,helper.getFunctionPtr());
	  auto ptr = loader.getRegister(). template get<pointer_t> ();
	    
	  auto func = inst.getFunction()->getPrgm()->getFunction(MiniMC::Support::getFunctionId (ptr));
	  assert(func->getParameters ().size () == helper.nbParams ());
	  std::vector<InRegister> regs;
	  for (size_t i = 0; i < helper.nbParams (); ++i) {
	    regs.push_back (st.stack.load (std::static_pointer_cast<MiniMC::Model::Variable> (helper.getParam (i))));
	  }
	  
	  st.push_frame (func->getVariableStackDescr ());
	  auto it = regs.begin();
	  for (auto& t : func->getParameters ()) {
	    st.stack.save (*it,t);
	    ++it;
	  }
	  auto resVar = helper.getRes ();
	  if (resVar) {
	    auto data = st.stack.getData();
	    data.ret  = std::static_pointer_cast<MiniMC::Model::Variable> (resVar).get();
	    st.stack.setData (data);
	  }
	  
	  
	  
	}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Ret,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Ret> helper (inst);
	  RegisterLoader val (st,helper.getValue());
	  MiniMC::Model::Variable_ptr storePlace = st.stack.getData().ret->shared_from_this();
	  assert(storePlace);
	  auto variableStackDescr = inst.getFunction()->getVariableStackDescr ();
	  assert(!st.stack.isModified());
	  st.pop_frame();
	  st.stack.save (val.getRegister(),storePlace);
	  
	}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Alloca,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Alloca> helper (inst);
	  MiniMC::Model::Value_ptr size = helper.getSize();
	  MiniMC::Model::Value_ptr storePlace = helper.getResult ();
	  assert(size->isConstant ());
	  assert(storePlace);
	  auto constant = std::static_pointer_cast<MiniMC::Model::IntegerConstant> (size);
	  pointer_t pointer;
	  auto data = st.stack.getData ();
	  assert(!MiniMC::Support::is_null (data.pointer));
	  pointer = st.heap.extend_obj (data.allocs,constant->getValue());
	  
	  InRegister reg (&pointer,sizeof(pointer));
	  auto vvar = std::static_pointer_cast<MiniMC::Model::Variable> (storePlace);
	  st.stack.save (reg,vvar);
	}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Load,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Load> helper (inst);
	  MiniMC::Model::Value_ptr res = helper.getResult();
	  auto size = res->getType()->getSize();
	  MiniMC::Model::Value_ptr address = helper.getAddress ();
	  RegisterLoader addrval (st,address);
	  auto pointer = addrval.getRegister ().template get<MiniMC::pointer_t> ();
	  
	  auto val = st.heap.read (pointer,size);
	  st.stack.save (val,std::static_pointer_cast<MiniMC::Model::Variable> (res));
	}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Assert,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assert> helper (inst);
	  RegisterLoader ass (st,helper.getAssert());
	  assert(ass.getRegister().getSize () == 1);
	  if (!ass.getRegister().template get <MiniMC::uint8_t> ()) {
	    throw MiniMC::Support::AssertViolated ();
	  }
	  
	}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Store,void> {
	void static execute (MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Store> helper (inst);
	  MiniMC::Model::Value_ptr value = helper.getValue();
	  auto size = value->getType()->getSize();
	  MiniMC::Model::Value_ptr address = helper.getAddress ();
	  RegisterLoader addrval (st,address);
	  RegisterLoader valueval (st,value);
	  auto pointer = addrval.getRegister ().template get<MiniMC::pointer_t> ();
	  
	  st.heap.write (pointer,valueval.getRegister());
	}
      };
      
    }
  }
}
