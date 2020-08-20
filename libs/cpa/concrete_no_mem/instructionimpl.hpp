#include "model/instructions.hpp"
#include "model/variables.hpp"
#include "support/div.hpp"
#include "support/rightshifts.hpp"
#include "support/exceptions.hpp"
#include "support/localisation.hpp"
#include "support/pointer.hpp"
#include "support/random.hpp"
#include "support/storehelp.hpp"

#include "register.hpp"
#include "stack.hpp"
#include "state.hpp"
#include "compimpl.hpp"
#include "castimpl.hpp"

namespace MiniMC {
  namespace CPA {
    namespace ConcreteNoMem {
      template<MiniMC::Model::InstructionCode c>
      class NotImplemented : public MiniMC::Support::Exception {
      public:
		NotImplemented () : MiniMC::Support::Exception (MiniMC::Support::Localiser{"Instruction '%1%' not implemented for this CPA"}.format (c)) {}
      };
      
      template<MiniMC::Model::InstructionCode opc,class S = void>
      struct ExecuteInstruction {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails&,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails&, const MiniMC::Model::Instruction& )  {
	  
		  throw NotImplemented<opc> ();
		}
      };
      
      template<MiniMC::Model::InstructionCode i,class T>
      struct TACExec {
		static OutRegister execute (const InRegister& left, const InRegister& right){
		  throw NotImplemented<i> ();
	
	  
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Add,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> () + right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Sub,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> ()  - right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };

      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Mul,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> ()  * right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::UDiv,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = MiniMC::Support::div(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::SDiv,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = MiniMC::Support::idiv(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Shl,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> () << right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::LShr,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = MiniMC::Support::lshr(left.template get<T> (),right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
      };
      
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::AShr,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = MiniMC::Support::ashr(left.template get<T> (), right.template get<T> ());
		  return OutRegister(hh,sizeof(T)); 
		}
      };

      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::And,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> () & right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };

      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Or,T> {
		static OutRegister execute (const InRegister& left, const InRegister& right) {
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
		  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> () | right.template get<T> ();
		  return OutRegister(hh,sizeof(T)); 
		}
      };
	  
      template<class T>
      struct TACExec<MiniMC::Model::InstructionCode::Xor,T> {
	static OutRegister execute (const InRegister& left, const InRegister& right) {
	  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
	  MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = left.template get<T> () ^ right.template get<T> ();
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
		  throw MiniMC::Support::Exception ("Bug");
		}
      }
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Skip,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails&,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails&, const MiniMC::Model::Instruction& )  {
		  
		  
		}
      };
	  
      class RegisterLoader {
      public:
		RegisterLoader (const State::StackDetails& details, const MiniMC::Model::Value_ptr& ptr) {
		  if(ptr->isConstant ()) {
			auto constant = std::static_pointer_cast<MiniMC::Model::Constant> (ptr);
			std::size_t size = ptr->getType()->getSize();
			
			std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[size]);;
			std::copy(constant->getData (),constant->getData ()+size,buffer.get());
	    
			reg = std::make_unique<InnerStateConst> (buffer,size);
		    
		  }
		  else if (ptr->isNonCompileConstant ()) {
			auto constant = std::static_pointer_cast<MiniMC::Model::AggregateNonCompileConstant> (ptr);
			std::size_t size = ptr->getType()->getSize();
			
			std::unique_ptr<MiniMC::uint8_t[]> buffer (new MiniMC::uint8_t[size]);;
			auto out = buffer.get();
			auto end = constant->end();
			for (auto it = constant->begin(); it != end; ++it) {
			  RegisterLoader rl (details,*it);
			  out = std::copy (reinterpret_cast<const uint8_t*> (rl.getRegister ().getMem ()),
							   reinterpret_cast<const uint8_t*> (rl.getRegister ().getMem ())+rl.getRegister().getSize (),
							   out);
			}
			reg = std::make_unique<InnerStateConst> (buffer,size);
		    
		  }
		  else {
			auto regi = (ptr->isGlobal() ? details.gstack : details.stack).load (std::static_pointer_cast<MiniMC::Model::Variable> (ptr));
			reg = std::make_unique<InnerStateIn> (regi);
			
		  }
		}

		const InRegister& getRegister () const {return reg->getReg ();}
	
      private:
	
		struct InnerState {
		  virtual ~InnerState () {}
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
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<opc> helper (inst);
		  RegisterLoader l (readFrom,helper.getLeftOp ());
		  RegisterLoader r (readFrom,helper.getRightOp ());
		  auto res = TypeRedirect<TACRedirect<opc>> (l.getRegister(),r.getRegister(),helper.getLeftOp ()->getType ());
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),res);
		  
		}
      };
	  
      template<MiniMC::Model::InstructionCode opc>
      struct ExecuteInstruction<opc,typename std::enable_if<MiniMC::Model::InstructionData<opc>::isCast>::type> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
					    MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
					    const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<opc> helper (inst);
		  auto& castee = helper.getCastee ();
		  auto& res =  helper.getResult ();
		  RegisterLoader l (readFrom,castee);
		  auto& casteeval =  l.getRegister ();
	  
		  auto resval = RedirectFrom<opc> (l.getRegister (), castee->getType(), res->getType ());
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
	  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::IntToPtr,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::IntToPtr> helper (inst);
		  auto& castee = helper.getCastee ();
		  auto& res =  helper.getResult ();
		  assert(castee->getType ()->getSize() == res->getType ()->getSize());
		  std::size_t size =  castee->getType ()->getSize();
		  
		  RegisterLoader l (readFrom,castee);
		  auto& casteeval =  l.getRegister ();
		  
		  std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[size]);
		  auto buf = reinterpret_cast<const MiniMC::uint8_t*> (casteeval.getMem());
		  std::copy (buf,buf+size,hh.get());
		  OutRegister resval (hh,size);
		  
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
		  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::PtrAdd,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::PtrAdd> helper (inst);
		  auto& value = helper.getValue ();
		  auto& addr =  helper.getAddress ();
		  auto& skipSize = helper.getSkipSize();
		  auto& res = helper.getResult();	  
		  RegisterLoader valueR (readFrom,value);
		  RegisterLoader addrR (readFrom,addr);
		  RegisterLoader skipSizeR (readFrom,skipSize);
	  
		  MiniMC::pointer_t ptr = addrR.getRegister().template get<MiniMC::pointer_t> ();
		  MiniMC::uint64_t skipCalc = 0;
		  assert(skipSizeR.getRegister().getSize () == valueR.getRegister().getSize ());
		  switch (skipSizeR.getRegister().getSize ()) {
		  case 1:
			skipCalc = skipSizeR.getRegister ().template get<MiniMC::uint8_t>()*valueR.getRegister ().template get<MiniMC::uint8_t>();
			break;
		  case 2:
			skipCalc =skipSizeR.getRegister ().template get<MiniMC::uint16_t>()*valueR.getRegister ().template get<MiniMC::uint16_t>();
			break;
		  case 4:
			skipCalc = skipSizeR.getRegister ().template get<MiniMC::uint32_t>()*valueR.getRegister ().template get<MiniMC::uint32_t>();
			break;
		  case 8:
			skipCalc = skipSizeR.getRegister ().template get<MiniMC::uint64_t>()*valueR.getRegister ().template get<MiniMC::uint64_t>();
			break;
		  default:
			throw MiniMC::Support::Exception (MiniMC::Support::Localiser("Weird size of skip  %1%").format (skipSizeR.getRegister().getSize ()));
		  }
		  auto nptr = MiniMC::Support::ptradd (ptr,skipCalc);
		  InRegister resval (&nptr,sizeof(nptr));
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (res),resval);
		  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::PtrEq> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::PtrEq> helper (inst);
		  RegisterLoader l (readFrom,helper.getLeftOp ());
		  RegisterLoader r (readFrom,helper.getRightOp ());
		  MiniMC::uint8_t res = l.getRegister ().template get<pointer_t> () == r.getRegister ().template get<pointer_t> (); 
		  InRegister resval (&res,sizeof(MiniMC::uint8_t));
	  
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),resval);
	  
		  
		}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::PtrToInt,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::PtrToInt> helper (inst);
		  auto& castee = helper.getCastee ();
		  auto& res =  helper.getResult ();
		  assert(castee->getType ()->getSize() == res->getType ()->getSize());
		  std::size_t size =  castee->getType ()->getSize();
	  
		  RegisterLoader l (readFrom,castee);
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
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<opc> helper (inst);
		  RegisterLoader l (readFrom,helper.getLeftOp ());
		  RegisterLoader r (readFrom,helper.getRightOp ());
		  auto res = TypeRedirect<CMPRedirect<opc>> (l.getRegister(),r.getRegister(),helper.getLeftOp ()->getType ());
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),res);
	  
		  
		}
      };
	  
      


      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Assign,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st,
									const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assign> helper (inst);
		  RegisterLoader val (readFrom,helper.getValue ());
		  auto resultvar = helper.getResult ();
		  doSave (st,std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ()),val.getRegister());
	  
		}
      };
	  
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::RetVoid,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  { 
		  assert(!st.stack.isModified());
		  assert(!st.stack.getData().ret);
		  st.pop_frame();
		}
      };
	  
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Call,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Call> helper (inst);
		  //Missing evaluation of parameters
		  RegisterLoader loader (readFrom,helper.getFunctionPtr());
		  auto ptr = loader.getRegister(). template get<pointer_t> ();
		  
		  auto func = inst.getFunction()->getPrgm()->getFunction(MiniMC::Support::getFunctionId (ptr));
		  assert(func->getParameters ().size () == helper.nbParams ());
		  std::vector<InRegister> regs;
		  for (size_t i = 0; i < helper.nbParams (); ++i) {
			regs.push_back (readFrom.stack.load (std::static_pointer_cast<MiniMC::Model::Variable> (helper.getParam (i))));
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
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::StackSave,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::StackSave> helper (inst);
		  auto data = st.stack.getData();
		  auto resVar = helper.getResult ();
		  InRegister reg (&data.allocs,sizeof(pointer_t));
		  st.stack.save (reg,std::static_pointer_cast<MiniMC::Model::Variable> (resVar));
		  auto ptr = st.heap.findSpace (0);
		  data.allocs = st.heap.make_obj (0,ptr);
		  st.stack.setData (data);
		}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::StackRestore,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::StackRestore> helper (inst);
		  RegisterLoader loader (readFrom,helper.getValue());
		  auto data = st.stack.getData();
		  st.heap.free_obj (data.allocs,true);
		  data.allocs = loader.getRegister(). template get<pointer_t> ();
		  auto ptr = st.heap.findSpace (0);
		  data.allocs = st.heap.make_obj (0,ptr);
		  st.stack.setData (data);
		}
      };

	  template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::MemCpy,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::MemCpy> helper (inst);
		  RegisterLoader source (readFrom,helper.getSource());
		  RegisterLoader target (readFrom,helper.getTarget());
		  RegisterLoader size (readFrom,helper.getSize());
		  
		  auto source_ptr = source.getRegister().template get<MiniMC::pointer_t> ();
		  auto target_ptr = target.getRegister().template get<MiniMC::pointer_t> ();
		  MiniMC::offset_t size_int = 0;
		  switch (size.getRegister ().getSize ()) {
		  case 1:
			size_int = size.getRegister ().template get<MiniMC::uint8_t> ();
			break;
		  case 2:
			size_int = size.getRegister ().template get<MiniMC::uint16_t> ();
			break;
		  case 4:
			size_int = size.getRegister ().template get<MiniMC::uint32_t> ();
			break;
		  case 8:
			size_int = size.getRegister ().template get<MiniMC::uint64_t> ();
			break;
		  }
		  
		  auto loaded = readFrom.heap.read (source_ptr,size_int);
		  st.heap.write (target_ptr,loaded);
		}
      };
	  
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Ret,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Ret> helper (inst);
		  RegisterLoader val (st,helper.getValue());
		  MiniMC::Model::Variable_ptr storePlace = readFrom.stack.getData().ret->shared_from_this();
		  assert(storePlace);
		  auto variableStackDescr = inst.getFunction()->getVariableStackDescr ();
		  assert(!st.stack.isModified());
		  st.pop_frame();
		  st.stack.save (val.getRegister(),storePlace);
		  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Alloca,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Alloca> helper (inst);
		  MiniMC::Model::Value_ptr size = helper.getSize();
		  MiniMC::Model::Value_ptr storePlace = helper.getResult ();
		  assert(size->isConstant ());
		  assert(storePlace);
		  auto constant = std::static_pointer_cast<MiniMC::Model::IntegerConstant<MiniMC::uint64_t>> (size);
		  MiniMC::loadHelper<MiniMC::uint64_t> sizeLoad (constant->getData(),sizeof(MiniMC::uint64_t));
		  pointer_t pointer;
		  auto data = readFrom.stack.getData ();
		  assert(!MiniMC::Support::is_null (data.allocs));
		  pointer = st.heap.extend_obj (data.allocs,sizeLoad);
		  InRegister reg (&pointer,sizeof(pointer));
		  auto vvar = std::static_pointer_cast<MiniMC::Model::Variable> (storePlace);
		  st.stack.save (reg,vvar);
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::FindSpace,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::FindSpace> helper (inst);
		  MiniMC::Model::Value_ptr storePlace = helper.getResult ();
		  assert(storePlace);
		  auto ptr = st.heap.findSpace (0);
		  InRegister reg (&ptr,sizeof(ptr));
		  auto vvar = std::static_pointer_cast<MiniMC::Model::Variable> (storePlace);
		  doSave (st,vvar,reg);
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Malloc,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Malloc> helper (inst);
		  MiniMC::Model::Value_ptr pointer = helper.getPointer ();
		  MiniMC::Model::Value_ptr size = helper.getSize ();
		  assert(size->getType()->getTypeID () == MiniMC::Model::TypeID::Integer);
		  assert(pointer->getType()->getTypeID () == MiniMC::Model::TypeID::Pointer);
		  RegisterLoader regPointer (readFrom,pointer);
		  RegisterLoader regSize (readFrom,size);
		  st.heap.make_obj (regSize.getRegister().template get<MiniMC::uint64_t> (),regPointer.getRegister().template get<MiniMC::pointer_t> ());
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Free,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Free> helper (inst);
		  MiniMC::Model::Value_ptr pointer = helper.getPointer ();
		  RegisterLoader regPointer (readFrom,pointer);
		  st.heap.free_obj (regPointer.getRegister().template get<MiniMC::pointer_t> ());
		}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Load,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Load> helper (inst);
		  MiniMC::Model::Value_ptr res = helper.getResult();
		  auto size = res->getType()->getSize();
		  MiniMC::Model::Value_ptr address = helper.getAddress ();
		  RegisterLoader addrval (readFrom,address);
		  auto pointer = addrval.getRegister ().template get<MiniMC::pointer_t> ();
		  auto val = readFrom.heap.read (pointer,size);
		  st.stack.save (val,std::static_pointer_cast<MiniMC::Model::Variable> (res));
		}
      };
	  
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Assert,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assert> helper (inst);
		  RegisterLoader ass (readFrom,helper.getAssert());
		  assert(ass.getRegister().getSize () == 1);
		  if (!ass.getRegister().template get <MiniMC::uint8_t> ()) {
			throw MiniMC::Support::AssertViolated ();
		  }
		  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Assume,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Assume> helper (inst);
		  RegisterLoader ass (readFrom,helper.getAssert());
		  assert(ass.getRegister().getSize () == 1);
		  if (!ass.getRegister().template get <MiniMC::uint8_t> ()) {
			throw MiniMC::Support::AssumeViolated ();
		  }
	  
		}
      };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::NegAssume,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::NegAssume> helper (inst);
		  RegisterLoader ass (readFrom,helper.getAssert());
		  assert(ass.getRegister().getSize () == 1);
		  if (ass.getRegister().template get <MiniMC::uint8_t> ()) {
			throw MiniMC::Support::AssumeViolated ();
		  }
	  
		}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Store,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Store> helper (inst);
		  MiniMC::Model::Value_ptr value = helper.getValue();
		  auto size = value->getType()->getSize();
		  MiniMC::Model::Value_ptr address = helper.getAddress ();
		  RegisterLoader addrval (readFrom,address);
		  RegisterLoader valueval (readFrom,value);
		  auto pointer = addrval.getRegister ().template get<MiniMC::pointer_t> ();
		  st.heap.write (pointer,valueval.getRegister());
		}
      };
      
      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Uniform,void> {
		inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
									MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  
		  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Uniform> helper (inst);
		  MiniMC::Model::Value_ptr value = helper.getResult();
		  MiniMC::Model::Value_ptr min = helper.getMin();
		  MiniMC::Model::Value_ptr max = helper.getMax();
		  auto size = value->getType()->getSize();
		  RegisterLoader minreg (readFrom,min);
		  RegisterLoader maxreg (readFrom,max);
		  assert(value->getType () ->getTypeID () == MiniMC::Model::TypeID::Integer);
		  MiniMC::Support::RandomNumber rand;
		  auto functor = [&]<typename T> () {
											 auto val = rand.template uniform<T> (minreg.getRegister ().template get<T> (),
																				  maxreg.getRegister ().template get<T> ());
											 auto nreg = InRegister (&val,sizeof(T));
											 st.stack.save (nreg,std::static_pointer_cast<MiniMC::Model::Variable> (value));
		  };


		  switch (size) {
		  case 1: 
		        functor.operator()<MiniMC::uint8_t>  ();
			break;
		  case 2: 
			functor.operator()<MiniMC::uint16_t> ();
			break;
		  case 4: 
			functor.operator()<MiniMC::uint32_t> ();
			break;
		  case 8: 
			functor.operator()<MiniMC::uint64_t> ();
			break;
		  default:
			assert(false);
		  }
		}
      };

	  struct VMData {
		const MiniMC::CPA::ConcreteNoMem::State::StackDetails* readFrom;
		MiniMC::CPA::ConcreteNoMem::State::StackDetails* st;
		void finalise () {
		  st->commit ();
		}
	  };
	  
	  struct ExecuteMap {
		template<MiniMC::Model::InstructionCode i>
		static void execute (VMData& data, const MiniMC::Model::Instruction& inst) {
		  ExecuteInstruction<i>::execute (*data.readFrom,*data.st,inst);
		}
	  };

      template<>
      struct ExecuteInstruction<MiniMC::Model::InstructionCode::Not,void> {
	inline static void execute (const MiniMC::CPA::ConcreteNoMem::State::StackDetails& readFrom,
				    MiniMC::CPA::ConcreteNoMem::State::StackDetails& st, const MiniMC::Model::Instruction& inst)  {
	  

	  
	  MiniMC::Model::InstHelper<MiniMC::Model::InstructionCode::Not> helper (inst);
	  auto type = helper.getOp ()->getType();
	  assert(type->getTypeID () == MiniMC::Model::TypeID::Integer);
	  
	  auto doNot =  [&]<typename T> () {
					    RegisterLoader op (readFrom,helper.getOp ());
					    std::unique_ptr<MiniMC::uint8_t[]>  hh (new MiniMC::uint8_t[sizeof(T)]);
					    MiniMC::saveHelper<T> (hh.get (),sizeof(T)) = ~(op.getRegister().template get<T> ());
					    OutRegister reg (hh,sizeof(T));
					    auto res = std::static_pointer_cast<MiniMC::Model::Variable> (helper.getResult ());
					    doSave (st,res,reg);
			    
			  };
	  
	  switch (type->getSize()) {
	  case 1:
	    doNot.operator()<MiniMC::uint8_t>  ();
	    break;
	  case 2:
	    doNot.operator()<MiniMC::uint16_t>  ();
	    break;
	  case 4:
	    doNot.operator()<MiniMC::uint32_t>  ();
	    break;
	  case 8:
	    doNot.operator()<MiniMC::uint64_t>  ();
	    break;
	  default:
	    throw MiniMC::Support::Exception ("Error");
	  }
	  
	}
      };
      
      
    }
  }
}
